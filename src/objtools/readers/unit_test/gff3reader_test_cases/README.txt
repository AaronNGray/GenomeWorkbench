To add additional test cases to this unit test:
--------------------------------------------------------------------------------
(1) Decide on a test name that has not been taken by another test, f.e. "dummy".
(2) Provide a gvf input file for the test, name it dummy.gff3, and put it in this
    directory.
(3) Provide an asn file that reflects the expected conversion of dummy.gff3. Name 
    it dummy.asn and also put it in this directory.
	
That's it - the unit test will scan this directory each time it runs, and thus
find and execute the new test automatically.
