
CREATE TABLE gain_resistor (
       sat TEXT,           -- Swarm A, B, or C
       probe INTEGER,      -- Probe 1, 2, or facplate (probe=3)
       rl REAL,            -- low resistor in Ohm
       rh REAL,            -- high resisotr in Ohm
       r3 REAL             -- 3rd gain resistor (only faceplate)
);
-- at low gain high and low resistors are coupled parallel:
-- Rlg=1/(1/rl + 1/rh), Rhg=rh

INSERT INTO gain_resistor VALUES
("A", 1, 67961.86, 3315608.0,       0.0),
("A", 2, 68341.76, 3315081.0,       0.0),
("A", 3,  1099.5,    11037.63, 100291.0),
("B", 1, 68222.2,  3305020.0,       0.0),
("B", 2, 68206.0,  3319532.0,       0.0),
("B", 3,  1100.0,    11000.0,  100000.0),
("C", 1, 67879.1,  3323814.0,       0.0),
("C", 2, 67997.4,  3313807.0,       0.0),
("C", 3,  1100.0,    11000.0,  100000.0);

CREATE TABLE tm2va (
       DACBits   INTEGER,   -- Probe 1 and 2 16 bit DACS
       VBmin_tm  INTEGER,   -- Min voltage/[TM_V]
       VpTM_ADC  REAL,      -- [V/TM] DAC
       VpTM_DAC  REAL       -- [V/TM] DAC
);
INSERT INTO tm2va VALUES
(16, -32768, 0.000152592547379986, 0.000152592547379986);

CREATE TABLE tm2amp (
    sat TEXT,
    probe INTEGER,
    slg REAL,
    shg  REAL,
    PRIMARY KEY (sat, probe)
) WITHOUT rowid;

DELETE FROM tm2amp;
INSERT INTO tm2amp SELECT 'A',1,(1.0/rl + 1.0/rh),1.0/rh FROM gain_resistor WHERE sat='A' AND probe=1;
INSERT INTO tm2amp SELECT 'A',2,(1.0/rl + 1.0/rh),1.0/rh FROM gain_resistor WHERE sat='A' AND probe=2;
INSERT INTO tm2amp SELECT 'B',1,(1.0/rl + 1.0/rh),1.0/rh FROM gain_resistor WHERE sat='B' AND probe=1;
INSERT INTO tm2amp SELECT 'B',2,(1.0/rl + 1.0/rh),1.0/rh FROM gain_resistor WHERE sat='B' AND probe=2;
INSERT INTO tm2amp SELECT 'C',1,(1.0/rl + 1.0/rh),1.0/rh FROM gain_resistor WHERE sat='C' AND probe=1;
INSERT INTO tm2amp SELECT 'C',2,(1.0/rl + 1.0/rh),1.0/rh FROM gain_resistor WHERE sat='C' AND probe=2;

UPDATE tm2amp SET (slg,shg)=((SELECT VpTM_DAC FROM tm2va)*slg, (SELECT VpTM_DAC FROM tm2va)*shg);

CREATE TABLE LPconst (
       probe_radius REAL,
       fp_area REAL
);
-- INSERT INTO LPconst VALUES(0.004, 0.06709664);
INSERT INTO LPconst VALUES(0.004, 0.0804);
