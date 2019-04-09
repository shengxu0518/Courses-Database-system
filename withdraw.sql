delimiter / 
DROP PROCEDURE IF EXISTS course_withdraw/
CREATE PROCEDURE course_withdraw (
	IN v_code varchar(15), 
    IN v_id int, 
    OUT output int)
BEGIN
	DECLARE grades varchar(2);
    DECLARE y int;
    DECLARE sem VARCHAR(2);
	DECLARE EXIT HANDLER FOR SQLEXCEPTION ROLLBACK;
	START TRANSACTION;
	IF (select exists(select * from transcript where StudId=v_id AND UoSCode=v_code))=0 THEN
		set output=0;
	ELSE
		IF (SELECT grade from transcript where StudId=v_id AND UoSCode=v_code) IS NULL THEN
			SELECT year INTO y from transcript where StudId=v_id AND UoSCode=v_code;
            SELECT Semester INTO sem from transcript where StudId=v_id AND UoSCode=v_code;
			DELETE FROM transcript WHERE  (StudId= v_id AND UoSCode= v_code); 
			UPDATE uosoffering SET Enrollment = Enrollment -1 WHERE (Semester=sem AND  Year=y  AND UoSCode= v_code);
			SET output =1;
		ELSE
			set output=2;
		end if;
	end if;
	COMMIT;
END /
delimiter ;

DROP table IF EXISTS class_status;
create table class_status(
	below_50 int DEFAULT 0
);
INSERT INTO class_status VALUES(0);

delimiter / 
DROP TRIGGER IF EXISTS enrollment_number/
CREATE TRIGGER enrollment_number AFTER UPDATE ON uosoffering 
FOR EACH ROW
	BEGIN
		IF new.Enrollment < new.MaxEnrollment * 0.5  THEN 
			UPDATE class_status SET below_50= 1;
		ELSE
			UPDATE class_status SET below_50= 0;
        END IF;
	END/
delimiter ;