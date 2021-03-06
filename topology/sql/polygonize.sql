-- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
-- 
-- PostGIS - Spatial Types for PostgreSQL
-- http://postgis.refractions.net
--
-- Copyright (C) 2011 Sandro Santilli <strk@keybit.net>
--
-- This is free software; you can redistribute and/or modify it under
-- the terms of the GNU General Public Licence. See the COPYING file.
--
-- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
--
-- Functions used to polygonize topology edges
--
-- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

--{
--
-- int Polygonize(toponame)
--
-- TODO: allow restricting polygonization to a bounding box
--
-- }{
CREATE OR REPLACE FUNCTION topology.polygonize(toponame varchar)
	RETURNS text
AS
$$
DECLARE
  sql text;
	rec RECORD;
  faces int;
BEGIN

	sql := 'SELECT (st_dump(st_polygonize(geom))).geom from '
         || quote_ident(toponame) || '.edge_data';

  faces = 0;
	FOR rec in EXECUTE sql LOOP
    BEGIN
      PERFORM topology.AddFace(toponame, rec.geom);
      faces = faces + 1;
    EXCEPTION
      WHEN OTHERS THEN
        RAISE WARNING 'Error registering face % (%)', rec.geom, SQLERRM;
    END;
	END LOOP;
  RETURN faces || ' faces registered';
END
$$
LANGUAGE 'plpgsql';
--} Polygonize(toponame)
