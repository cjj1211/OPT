USE optimus;
START TRANSACTION;

alter table TreatmentRecord add  IsCompress TINYINT NOT NULL DEFAULT 1;

alter table ChannelPlan add DetectorTranslateX INT NOT NULL DEFAULT 0;
alter table ChannelPlan add DetectorTranslateY INT NOT NULL DEFAULT 0;
alter table ChannelPlan add DetectorTranslateZ INT NOT NULL DEFAULT 0;
alter table ChannelPlan add DetectorRotateX INT NOT NULL DEFAULT 0;
alter table ChannelPlan add DetectorRotateY INT NOT NULL DEFAULT 0;
alter table ChannelPlan add DetectorRotateZ INT NOT NULL DEFAULT 0;

alter table ChannelPlan add ProbeTranslateX INT NOT NULL DEFAULT 0;
alter table ChannelPlan add ProbeTranslateY INT NOT NULL DEFAULT 0;
alter table ChannelPlan add ProbeTranslateZ INT NOT NULL DEFAULT 0;
alter table ChannelPlan add ProbeRotateX INT NOT NULL DEFAULT 0;
alter table ChannelPlan add ProbeRotateY INT NOT NULL DEFAULT 0;
alter table ChannelPlan add ProbeRotateZ INT NOT NULL DEFAULT 0;

alter table ChannelPlan add AtlasMapType VARCHAR(256) NOT NULL DEFAULT "";
COMMIT;