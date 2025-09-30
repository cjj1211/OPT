USE optimus;
START TRANSACTION;
alter table TreatmentRecord add Note varchar(1024);
alter table TreatmentRecord add SaveFileType varchar(256);

UPDATE treatmentrecord set SaveFileType="rhd"  where SaveFileType="" or SaveFileType is null;

COMMIT;