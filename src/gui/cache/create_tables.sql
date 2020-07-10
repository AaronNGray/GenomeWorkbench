CREATE TABLE cache_data
(
   cache_key     varchar(256) NOT NULL,
   version       int          NOT NULL,
   subkey        varchar(256) NOT NULL,
   
   data          image        NULL
);

CREATE UNIQUE INDEX cache_data_pk ON cache_data(cache_key, version, subkey);

CREATE TABLE cache_attr
(
   cache_key        varchar(256) NOT NULL,
   version          int          NOT NULL,
   subkey           varchar(256) NOT NULL,
   
   cache_timestamp  int
);

CREATE UNIQUE INDEX cache_attr_pk ON cache_data(cache_key, version, subkey);

