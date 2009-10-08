-- SQL tables needed for the pgsql-backend of libtmrm
-- 
-- TODO: Add some indexes to speed things up!
--
-- This schema has some major problems:
--  * The _bottom_ proxy should be created by libtmrm!
--  * Literals should not be part of the property table. Create a
--    separate table instead (lessons learned from RDF)

CREATE TABLE proxy (
    id serial,
    hash VARCHAR(200),
    PRIMARY KEY (id)
);

CREATE TABLE property (
    proxy INTEGER NOT NULL,
    key INTEGER NOT NULL,
    value INTEGER,    
    value_literal TEXT,
    datatype TEXT,
    FOREIGN KEY (proxy) REFERENCES proxy(id),
    FOREIGN KEY (key) REFERENCES proxy(id),
    FOREIGN KEY (value) REFERENCES proxy(id)
);

INSERT INTO proxy (id) VALUES (0);
INSERT INTO property (proxy, key, value) VALUES (0, 0, 0);

