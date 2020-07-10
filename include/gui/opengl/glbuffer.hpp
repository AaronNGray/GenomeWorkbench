#ifndef GUI_OPENGL___GL_BUFFER__HPP
#define GUI_OPENGL___GL_BUFFER__HPP

/*  $Id: glbuffer.hpp 42898 2019-04-24 22:16:58Z katargir $
 * ===========================================================================
 *
 *                            PUBLIC DOMAIN NOTICE
 *               National Center for Biotechnology Information
 *
 *  This software/database is a "United States Government Work" under the
 *  terms of the United States Copyright Act.  It was written as part of
 *  the author's official duties as a United States Government employee and
 *  thus cannot be copyrighted.  This software/database is freely available
 *  to the public for use. The National Library of Medicine and the U.S.
 *  Government have not placed any restriction on its use or reproduction.
 *
 *  Although all reasonable efforts have been taken to ensure the accuracy
 *  and reliability of the software and data, the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties, express or implied, including
 *  warranties of performance, merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors:  Bob Falk
 *
 * File Description:
 *
 * glBuffer contains an interfaces for OpenGL buffers that allows buffers 
 * compatible with different versions of OpenGL to be created and allows
 * the same buffer to be used to render in different ways. 
 *
 * All the buffers have essentially the same primary interface which is 
 * presented abstractly in IGlBuffer.  All concreate buffers are then derived
 * from CGlBuffer which adds some size and index information common to the 
 * concrete buffer subclasses.
 *
 * The CGlBuffer11 subclass implements the buffer using an STL vector.  This 
 * can be made availabe for rendering using glVertexPointer, glColorPointer etc.
 *
 * The CGlBuffer20 subcalss implements buffers using OpenGL VBOs available on 
 * OpenGL 1.5+ and OpenglES 2.0.  
 *
 * The class CGlRenderBuffer implements the abastract interace and simply forwards
 * all function calls needing buffer access to a subclass of CGlBuffer which
 * must be provided as a parameter to CGlRenderBuffer.  This implementation allows
 * buffers subclassed from CGlBuffer to be used in many different rendering tasks
 * and have offset and stride information vary based on what is being rendered.
 *    
 * Note that buffers are setup for rendering by OpenGL version-specific classes
 * derived from CGlVboGeom. 
 *
 *
 *    IGlBuffer<T>  
 *     ^      ^
 *     |      |____________________
 *     |                          |
 *     |                          |
 *  CGlRenderBuffer<T>O-------CGlBuffer<T>
 *                              ^    ^
 *                              |    |
 *                        ______|    |__________
 *                        |                     |
 *                        |                     |
 *                   CGlBuffer11<T>         CGlBuffer20<T>
 */

#include <corelib/ncbiobj.hpp>
#include <gui/opengl.h>
//#include <gui/gui.hpp>

#include <gui/opengl/globject.hpp>
#include <gui/opengl/glutils.hpp>

/** @addtogroup GUI_OPENGL
 *
 * @{
 */

BEGIN_NCBI_SCOPE


//////////////////////////////////////////////////////////////////////////////
/// IGlBuffer
/// This class provides and abstract interface for an OpenGL buffer.  The
/// buffer implementation may be compliant with different versions of OpenGL
/// but must implement at least these functions.  (Derived buffers may also
/// support more functions for programs that declare them explicitely).
/// Note that some functions may be implemented as a no-op in a subclass
/// if they do bookkeeping or cleanup not required on all platforms.
///
/// An OpenGL buffer can hold vertex data, vertex attribute data, or indices
/// into other vertex buffers.  The underlying implmentation for the buffers
/// will be as Vertex Buffer Objects (VBOs) for versions of OpenGL > 1.4, and
/// as vetex arrays otherwise.  The buffer commands map to the OpenGL VBO
/// coammands so you can see documentation there.
///
template<typename T>
class IGlBuffer
{
public:
    typedef T TBufferType;

public:
    ///
    IGlBuffer() {}

    ///
    virtual ~IGlBuffer() {}

    /// Do any needed buffer clean-up 
    virtual void Destroy() = 0;
    /// Create and, optionally, initialize the buffer
    /// @param size
    ///   Number of data elements (of type T) in buffer
    /// @param data
    ///   Location from which to copy the data (if NULL, only reserves memory)
    /// @param usage
    ///   Hint to the graphics system as to how the buffer will be used - one
    ///   of 9 possible: GL_{STATIC, DYNAMIC, STREAM}_{READ, COPY, DRAW}
    virtual void BufferData(GLsizei num_elements, const T* data, GLenum usage) = 0;

    /// Return number of elements currently in use (for rendering)
    virtual GLsizei GetActiveBufferSize() const = 0;

    /// Return a pointer to the beginning of the buffer to allow direct
    /// memory reads/writes.  Must unmap before rendering.
    /// @param access
    ///   Type of access {GL_READ_ONLY, GL_WRITE_ONLY, GL_READ_WRITE}
    virtual T* MapBuffer(GLenum access) = 0;
    /// Unmap data (pointer may returned by MapBuffer becomes invalid).
    /// Note that for regular array implementation, this may have no effect.
    virtual bool UnmapBuffer() = 0;
    /// Return pointer to buffer if currently mapped, or NULL otherwise
    virtual T* GetMappedPtr() = 0;
    /// Return true if buffer is currently mapped
    virtual bool IsMapped() const = 0;

    /// Return entry in the buffer ONLY if it is mapped (if IsMapped()==true)
    virtual T& operator[](GLsizei idx) = 0;
    /// Return a copy of the buffer (0..m_ActiveSize). Mmaps/unmaps if needed
    /// If buffer already mapped, usage must be GL_READ_ONLY or GL_READ_WRITE
    virtual void GetData(vector<T>& data) = 0;

    /// Bind buffer - glBindBuffer(target,id) for non-array implementations
    virtual void BindBuffer() = 0;
    /// Release buffer - glBindBuffer(target,0) for non-array implementations
    virtual void ReleaseBuffer() = 0;

    /// Get allocated buffer size (size in use for rendering may be less)
    virtual GLsizei GetSize() const = 0;
};




//////////////////////////////////////////////////////////////////////////////
/// CGlBuffer
/// Concrete buffers (to store vertices, vertex attributes, or indices to 
/// vertex buffers) are derived from this class.  This class implements
/// functions common to both 1.1 and 2.0 implementations, namely the
/// buffer size, buffer pointer (when mapped) and current index.
template<typename T>
class CGlBuffer 
    : public CGlObject
    , public IGlBuffer<T>
{
public:
    typedef T TBufferType;

public:
    /// Sets buffer type but does not allocate buffer yet
    /// @param target
    ///  GL_ARRAY_BUFFER (vertex attributes), GL_ELEMENT_ARRAY_BUFFER (indices)
    CGlBuffer(GLenum target);

    /// Get number of elements to be drawn
    GLsizei GetActiveBufferSize() const {return m_ActiveSize;}

    /// Get pointer to mapped buffer (NULL if buffer not currently mapped)
    T* GetMappedPtr() { return m_BufferData; }

    /// Returns true if buffer data has been mapped
    bool IsMapped() const { return (m_BufferData != NULL); }

    /// Bind buffer - subclass to implement if needed (2.0 only)
    virtual void BindBuffer() {}

    /// Release buffer - subclass to implement if needed (2.0 only)
    virtual void ReleaseBuffer() {}

    /// Get allocated buffer size (# of elements T)
    GLsizei GetSize() const { return m_BufferSize; }
   
    /// Number of elements to use when drawing (m_BufferSize may be larger)
    GLsizei m_ActiveSize;

protected:

    /// GL_ARRAY_BUFFER (for vertex attributes) or:
    /// GL_ELEMENT_ARRAY_BUFFER (for array indices)
    GLenum m_TargetBuffer;

    /// Number of elements of type "T" that can fit in buffer
    GLsizei m_BufferSize;

    /// Pointer to buffer data in memeory when buffer is mapped into memory
    /// via MapBuffer, otherwise NULL.  
    TBufferType*  m_BufferData;  
};

template<class T>
CGlBuffer<T>::CGlBuffer(GLenum target) 
: m_TargetBuffer(target)
, m_BufferSize(0)
, m_ActiveSize(0)
, m_BufferData(NULL)
{
}

//////////////////////////////////////////////////////////////////////////////
/// CGlRenderBuffer
/// This is a thin wrapper for buffer objects derived from CGlBuffer. 
/// It's only purpose is to encapsulate rendering information that determines
/// how the buffer is used in rendering rather than what data is in the buffer.
/// Having this wrapper means that the same buffer can be used in multiple
/// rendering nodes and that changing how the buffer is used for rendering
/// in one node will not effect the rendering of another node using the same
/// buffer. 
/// 
/// The parameters here are not for OpenGL options like shading or line width
/// since those are all set by rendering nodes, but rather for array access like
/// stride and offset.  We may have a buffer of lines for example where
/// one node may want to render every line and another every other line, and 
/// that diference would be captured here.
template<typename T>
class CGlRenderBuffer : public IGlBuffer<T> {
public:
    /// Ctor - can't be used until m_Buffer is set
    CGlRenderBuffer() : m_DrawEnabled(false), m_Stride(0), m_Offset(0) {}

    /// Ctor that sets buffer
    CGlRenderBuffer( CRef<CGlBuffer<T> > buf ) 
        : m_Buffer(buf)
        , m_DrawEnabled(true)
        , m_Stride(0)
        , m_Offset(0) {}

    /// Ctor that sets buffer
    CGlRenderBuffer( CGlBuffer<T>* buf ) 
        : m_Buffer()
        , m_DrawEnabled(true)
        , m_Stride(0)
        , m_Offset(0) {}

    /// buffer is held as cref so not deleted
    virtual ~CGlRenderBuffer() {}

    ///
    /// Methods specific to this class (not forwarded to underlying buffer)
    ///

    /// Set/Get the underlying buffer
    void SetBuffer( CGlBuffer<T>* buf ) { m_Buffer.Reset(buf);  m_DrawEnabled = true;}
    CRef<CGlBuffer<T> >& GetBuffer() { return m_Buffer; }

    /// Set/get the stride (number of bytes to skip between elements)
    /// A stride of 0 assumes elements are tightly packed (adjacent)
    void SetStride(GLsizei s) {m_Stride = s;}
    GLsizei GetStride() const { return m_Stride; }

    /// Set/get the initial byte offset (index of first element to render)
    void SetOffset(GLsizei offset) { m_Offset = offset; }
    GLsizei GetOffset() const { return m_Offset; }

    /// If true, rendering will use this buffer if it is the correct size.
    /// This param can 'turn off' a buffer that has been allocated and filled
    void SetDrawEnabled(bool b) { m_DrawEnabled = b; }
    bool GetDrawEnabled() const { return m_DrawEnabled; }

    ///
    /// Methods forwarded to underlying implementation buffer. See documentation
    /// in IGlBuffer or concrete subclasses.
    ///

    void Destroy() { m_Buffer->Destroy(); }
    void BufferData(GLsizei num_elements, const T* data, GLenum usage) 
        { m_Buffer->BufferData(num_elements, data, usage); }
    
    GLsizei GetActiveBufferSize() const { return m_Buffer->GetActiveBufferSize(); }

    T* MapBuffer(GLenum access) { return m_Buffer->MapBuffer(access); }
    bool UnmapBuffer() { return m_Buffer->UnmapBuffer(); }
    T* GetMappedPtr() { return m_Buffer->GetMappedPtr(); }
    bool IsMapped() const { return m_Buffer->IsMapped(); }

    T& operator[](GLsizei idx) { return (*m_Buffer)[idx]; }
    void GetData(vector<T>& data) { m_Buffer->GetData(data); }

    void BindBuffer() { m_Buffer->BindBuffer(); }
    void ReleaseBuffer() { m_Buffer->ReleaseBuffer(); }

    GLsizei GetSize() const { return m_Buffer->GetSize(); }


protected:
    /// Underlying concreate implementation buffer
    CRef<CGlBuffer<T> > m_Buffer;

    /// Flag to allow renderers to 'turn off' a buffer during a
    /// rendering pass.
    bool m_DrawEnabled;

    /// Number of bytes between elements to render (tightly packed=>0)
    GLsizei m_Stride;
    /// Number of bytes from the front of the buffer to the first element
    GLsizei m_Offset;
};



//////////////////////////////////////////////////////////////////////////////
/// CGlBuffer20
/// Implement the abstract buffer interface based on OpenGL vertex buffer 
/// objects.  This should be how the buffers are most often used since this
/// has been available since OpenGL 1.5.  This does not use any buffer 
/// features added after OpenGL 2.0.
///
template<typename T>
class CGlBuffer20 : public CGlBuffer<T>
{
public:
    /// Sets buffer type but does not allocate buffer yet
    /// @param target
    ///  GL_ARRAY_BUFFER (vertex attributes), GL_ELEMENT_ARRAY_BUFFER (indices)
    CGlBuffer20(GLenum target);

    /// destroy buffer
    virtual ~CGlBuffer20() { Destroy(); }

    /// Delete buffer data.  May be re-created via BufferData()
    virtual void Destroy();

    /// Create and, optionally, initialize the buffer
    /// @param size
    ///   Number of data elements (of type T) in buffer
    /// @param data
    ///   Location from which to copy the data (if NULL, only reserves memory)
    /// @param usage
    ///   Hint to the graphics system as to how the buffer will be used - one
    ///   of 9 possible: GL_{STATIC, DYNAMIC, STREAM}_{READ, COPY, DRAW}
    void BufferData(GLsizei num_elements, const T* data, GLenum usage);

    /// Map the data into memory for direct (c++ pointer) access.  (Later
    /// versions of OpenGL also support a 'glMapBufferRange')
    /// @param access
    ///   Type of access {GL_READ_ONLY, GL_WRITE_ONLY, GL_READ_WRITE}
    T* MapBuffer(GLenum access);    

    /// Unmap buffer data in memory.  Error if not currently mapped.
    /// Buffer should not be mapped while rendering.
    bool UnmapBuffer();

    /// Access an entry in the buffer ONLY if it is mapped (if IsMapped()==true)
    T& operator[](GLsizei idx);
    
    /// Return a copy of the buffer (0..m_ActiveSize) - maps/unmaps if needed
    /// If buffer already mapped, usage must be GL_READ_ONLY or GL_READ_WRITE
    void GetData(vector<T>& data);

    /// Bind buffer - glBindBuffer(target, id)
    void BindBuffer();
    /// Release buffer - glBindBuffer(target, 0)
    void ReleaseBuffer();
   
protected:
    /// OpenGL id for the buffer
    GLuint m_GlBufferId;

    /// OpenGL usage (one of): GL_{STATIC, DYNAMIC, STREAM}_{READ, COPY, DRAW}
    GLenum m_Usage;

private:
    CGlBuffer20(const CGlBuffer20<T>&);
    CGlBuffer20<T>& operator=(const CGlBuffer20<T>&);
};


template<class T>
CGlBuffer20<T>::CGlBuffer20(GLenum target) 
: CGlBuffer<T>(target)
, m_GlBufferId(GL_ZERO)
, m_Usage(GL_ZERO)
{
}

template<class T>
void CGlBuffer20<T>::Destroy()
{
    if (glIsBuffer(m_GlBufferId)) {
        glDeleteBuffers(1, &m_GlBufferId);
    }

    if (this->m_BufferData != NULL) {
        UnmapBuffer();
    }
    
    m_GlBufferId = 0;
    this->m_BufferSize = 0;
    this->m_ActiveSize = 0;
    //this->m_BufferData = NULL;
}

template<class T>
void CGlBuffer20<T>::BufferData(GLsizei size, const T* data, GLenum usage)
{
    if (this->m_BufferData != NULL)
        LOG_POST(Warning << "Warning - reallocating buffer while buffer is mapped.");

    // If the buffer has not yet been created (or has been previously
    // destroyed:
    if (m_GlBufferId == 0)
        glGenBuffers(1, &m_GlBufferId);

    m_Usage = usage;

    glBindBuffer(this->m_TargetBuffer, m_GlBufferId);
    glBufferData(this->m_TargetBuffer, size*sizeof(T), data, usage);
    glBindBuffer(this->m_TargetBuffer, 0);

    this->m_BufferSize = size;
    this->m_ActiveSize = this->m_BufferSize;
}

template<class T>
T* CGlBuffer20<T>::MapBuffer(GLenum access)
{ 
    CGlUtils::CheckGlError();
    if (this->m_BufferSize == 0) {
        LOG_POST(Error << "Error - attempt to map openGL buffer prior to buffer creation");
        return NULL;
    }
    // already mapped...
    if (this->m_BufferData != NULL) {
        LOG_POST(Warning << "Warning - attempt to map openGL buffer that is already mapped");
        return this->m_BufferData;
    }
   
    glBindBuffer(this->m_TargetBuffer, m_GlBufferId);
    this->m_BufferData = (T*)glMapBuffer(this->m_TargetBuffer, access);
    glBindBuffer(this->m_TargetBuffer, 0);
    CGlUtils::CheckGlError();

    return this->m_BufferData;
}

template<class T>
bool CGlBuffer20<T>::UnmapBuffer()
{
   glBindBuffer(this->m_TargetBuffer, m_GlBufferId);
   bool b = glUnmapBuffer(this->m_TargetBuffer);

   this->m_BufferData = NULL;

   // system error - data store became invalid while mapped.
   // Must re-initialize.
   if (!b) {
       LOG_POST(Warning << "Warning - error unmapping buffer.");
       BufferData(this->m_BufferSize, NULL, m_Usage);
   }

   glBindBuffer(this->m_TargetBuffer, 0);

   return b;
}

template<class T>
T& CGlBuffer20<T>::operator[](GLsizei idx) 
{ 
    _ASSERT(idx < this->m_BufferSize);
    return *(this->m_BufferData + idx); 
}

template<class T>
void CGlBuffer20<T>::GetData(vector<T>& data)
{    
    if (this->m_ActiveSize == 0)
        return;

    data.clear();

    // Copy active section of buffer into 'data'.  Map buffer if needed.
    // If buffer is already mapped, it will need to have been mapped with
    // usage GL_READ_ONLY or GL_READ_WRITE.
    if (this->m_BufferData == NULL) {
        MapBuffer(GL_READ_ONLY);

        for (GLsizei i=0; i < this->m_ActiveSize; ++i)
            data.push_back((*this)[i]);

        UnmapBuffer();
    }
    else {
        for (GLsizei i=0; i < this->m_ActiveSize; ++i)
            data.push_back((*this)[i]);
    }
}

template<class T>
void CGlBuffer20<T>::BindBuffer()
{
    glBindBuffer(this->m_TargetBuffer, m_GlBufferId);
}

template<class T>
void CGlBuffer20<T>::ReleaseBuffer()
{
    glBindBuffer(this->m_TargetBuffer, 0);
}



//////////////////////////////////////////////////////////////////////////////
/// CGlBuffer11
/// Implement abstract buffer interface using simple STL array.  Functions for
/// updating the array still have same interface of course as OpenGL VBOs here
/// to allow easy swapping.  Rendering of the arrays can still be done using
/// glVertexPointer, glColorPointer, etc.
///
template<typename T>
class CGlBuffer11 : public CGlBuffer<T>
{
public:
    /// Sets buffer type but does not allocate buffer yet
    /// @param target
    ///  GL_ARRAY_BUFFER (vertex attributes), GL_ELEMENT_ARRAY_BUFFER (indices)
    CGlBuffer11(GLenum target);

    /// destroy buffer
    virtual ~CGlBuffer11() { Destroy(); }

    /// Delete/clear buffer data
    virtual void Destroy();

    /// Create and, optionally, initialize the buffer
    /// @param num_elements
    ///   Number of data elements (of type T) in buffer
    /// @param data
    ///   Location from which to copy the data (if NULL, only reserves memory)
    /// @param usage
    ///   Hint to the graphics system as to how the buffer will be used - one
    ///   of 9 possible: GL_{STATIC, DYNAMIC, STREAM}_{READ, COPY, DRAW}
    void BufferData(GLsizei num_elements, const T* data, GLenum usage);

    /// Map the data into memory for direct (c++ pointer) access.  (Later
    /// versions of OpenGL also support a 'glMapBufferRange')
    /// @param access
    ///   Type of access {GL_READ_ONLY, GL_WRITE_ONLY, GL_READ_WRITE}
    T* MapBuffer(GLenum access);    

    /// Unmap buffer data in memory.  Error if not currently mapped.
    /// Buffer should not be mapped while rendering.
    bool UnmapBuffer() { this->m_BufferData = NULL; return true; }

    /// Access an entry in the buffer ONLY if it is mapped (if IsMapped()==true)
    T& operator[](GLsizei idx);
    
    /// Return a copy of the buffer (0..m_ActiveSize) - maps/unmaps if needed
    /// If buffer already mapped, usage must be GL_READ_ONLY or GL_READ_WRITE
    void GetData(vector<T>& data);

protected:
    /// The underlying array
    std::vector<T> m_Buffer;

private:
    CGlBuffer11(const CGlBuffer11<T>&);
    CGlBuffer11<T>& operator=(const CGlBuffer11<T>&);
};


template<class T>
CGlBuffer11<T>::CGlBuffer11(GLenum target) 
: CGlBuffer<T>(target)
{
}


template<class T>
void CGlBuffer11<T>::Destroy()
{
    m_Buffer.clear();

    this->m_BufferSize = 0;
    this->m_ActiveSize = 0;
    this->m_BufferData = NULL;
}

template<class T>
void CGlBuffer11<T>::BufferData(GLsizei size, const T* data, GLenum usage)
{
    if (this->m_BufferData != NULL)
        LOG_POST(Warning << "Warning - re-allocating buffer while buffer is mapped.");

    //m_Usage = usage;

    m_Buffer.clear();
    m_Buffer.resize(size);
    if (data != NULL) {
        T* buf_data = &m_Buffer[0];
        memcpy(buf_data, data, size*sizeof(T));
    }

    this->m_BufferSize = size;
    this->m_ActiveSize = this->m_BufferSize;
}

template<class T>
T* CGlBuffer11<T>::MapBuffer(GLenum access)
{
    if (this->m_BufferSize == 0) {
        LOG_POST(Error << "Error - attempt to map OpenGL buffer prior to buffer creation");
        return NULL;
    }
    // already mapped...
    if (this->m_BufferData != NULL) {
        LOG_POST(Warning << "Warning - attempt to map openGL buffer that is already mapped");
        return this->m_BufferData;
    }
      
    this->m_BufferData = &m_Buffer[0];

    return this->m_BufferData;
}

template<class T>
T& CGlBuffer11<T>::operator[](GLsizei idx) 
{ 
    _ASSERT(idx < this->m_BufferSize);
    return m_Buffer[idx];
}

template<class T>
void CGlBuffer11<T>::GetData(vector<T>& data)
{    
    if (this->m_ActiveSize == (GLsizei)m_Buffer.size()) {
        data = m_Buffer;
        return;
    }

    data.clear();
    if (this->m_ActiveSize > 0) {
        data.insert(data.begin(), m_Buffer.begin(), m_Buffer.begin()+this->m_ActiveSize);
    }

    return;
}

END_NCBI_SCOPE

/* @} */

#endif  // GUI_OPENGL___GL_BUFFER__HPP
