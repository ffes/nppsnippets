-----------------------------------------------------------------------------
-- This is provided as a reference and not used by the plugin              --
-----------------------------------------------------------------------------

BEGIN TRANSACTION;
DROP TABLE IF EXISTS Languages;
CREATE TABLE Languages (
	LangID INTEGER PRIMARY KEY NOT NULL,
	Name TEXT NOT NULL
);
INSERT INTO Languages VALUES (0,'Normal Text');
INSERT INTO Languages VALUES (1,'PHP');
INSERT INTO Languages VALUES (2,'C');
INSERT INTO Languages VALUES (3,'C++');
INSERT INTO Languages VALUES (4,'C#');
INSERT INTO Languages VALUES (5,'Objective-C');
INSERT INTO Languages VALUES (6,'Java');
INSERT INTO Languages VALUES (7,'Resource file');
INSERT INTO Languages VALUES (8,'HTML');
INSERT INTO Languages VALUES (9,'XML');
INSERT INTO Languages VALUES (10,'Makefile');
INSERT INTO Languages VALUES (11,'Pascal');
INSERT INTO Languages VALUES (12,'Batch');
INSERT INTO Languages VALUES (13,'MS INI file');
INSERT INTO Languages VALUES (14,'MS-DOS Style');
INSERT INTO Languages VALUES (15,'User-Defined');
INSERT INTO Languages VALUES (16,'ASP');
INSERT INTO Languages VALUES (17,'SQL');
INSERT INTO Languages VALUES (18,'VB');
INSERT INTO Languages VALUES (19,'JavaScript');
INSERT INTO Languages VALUES (20,'CSS');
INSERT INTO Languages VALUES (21,'Perl');
INSERT INTO Languages VALUES (22,'Python');
INSERT INTO Languages VALUES (23,'Lua');
INSERT INTO Languages VALUES (24,'TeX');
INSERT INTO Languages VALUES (25,'Fortran');
INSERT INTO Languages VALUES (26,'Shell');
INSERT INTO Languages VALUES (27,'Flash actionscript');
INSERT INTO Languages VALUES (28,'NSIS');
INSERT INTO Languages VALUES (29,'TCL');
INSERT INTO Languages VALUES (30,'LISP');
INSERT INTO Languages VALUES (31,'Scheme');
INSERT INTO Languages VALUES (32,'Assembly');
INSERT INTO Languages VALUES (33,'Diff');
INSERT INTO Languages VALUES (34,'Properties');
INSERT INTO Languages VALUES (35,'PostScript');
INSERT INTO Languages VALUES (36,'Ruby');
INSERT INTO Languages VALUES (37,'Smalltalk');
INSERT INTO Languages VALUES (38,'VHDL');
INSERT INTO Languages VALUES (39,'KIXtart');
INSERT INTO Languages VALUES (40,'AutoIt');
INSERT INTO Languages VALUES (41,'Caml');
INSERT INTO Languages VALUES (42,'Ada');
INSERT INTO Languages VALUES (43,'Verilog');
INSERT INTO Languages VALUES (44,'Matlab');
INSERT INTO Languages VALUES (45,'Haskell');
INSERT INTO Languages VALUES (46,'INNO');
INSERT INTO Languages VALUES (48,'CMake');
INSERT INTO Languages VALUES (49,'YAML');
INSERT INTO Languages VALUES (50,'COBOL');
INSERT INTO Languages VALUES (51,'Gui4Cli');
INSERT INTO Languages VALUES (52,'D');
INSERT INTO Languages VALUES (53,'PowerShell');
INSERT INTO Languages VALUES (54,'R');
INSERT INTO Languages VALUES (55,'JSP');
INSERT INTO Languages VALUES (56,'CoffeeScript');
INSERT INTO Languages VALUES (57,'JSON');
INSERT INTO Languages VALUES (58,'JavaScript');
INSERT INTO Languages VALUES (59,'Fortran 77');
INSERT INTO Languages VALUES (60,'BaanC');
INSERT INTO Languages VALUES (61,'Motorola S-Record binary data');
INSERT INTO Languages VALUES (62,'Intel HEX binary data');
INSERT INTO Languages VALUES (63,'Tektronix extended HEX binary data');
COMMIT;
