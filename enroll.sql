delimiter /    
DROP PROCEDURE IF EXISTS course_enroll/
CREATE PROCEDURE course_enroll(
	IN v_code varchar(15), 
	IN v_id int, 
    IN Semes char(5), 
    IN s_year int, 
    OUT output INT)
BEGIN
DECLARE  max_number int;
DECLARE  enrollment_num int;
DECLARE  prerequisite_num int;
DECLARE prerequisite_pass int;
DECLARE EXIT HANDLER FOR SQLEXCEPTION ROLLBACK;	
START TRANSACTION;
	IF (v_code in (SELECT UoSCode FROM transcript WHERE v_id = StudId )) THEN 
		SET output =0;
	ELSE 
		SET max_number = (SELECT MaxEnrollment FROM uosoffering WHERE v_code = UosCode AND Semes=Semester AND s_year = Year);
		SET enrollment_num = (SELECT Enrollment FROM uosoffering WHERE v_code = UosCode AND Semes=Semester AND s_year = Year);
		SET prerequisite_num = (SELECT COUNT(UoSCode) FROM requires WHERE UoSCode = v_code);
		SET prerequisite_pass = (SELECT COUNT(*) FROM (transcript natural join (SELECT PrereqUoSCode as UoSCode FROM requires WHERE UoSCode = v_code) AS prerequisite_courses) WHERE grade IS NOT NULL AND StudId =v_id);
		IF (prerequisite_pass = prerequisite_num) THEN 
			IF (enrollment_num < max_number) THEN 
				UPDATE uosoffering SET Enrollment =Enrollment + 1 WHERE (Semester=Semes AND  Year=s_year  AND UoSCode= v_code);
				INSERT INTO transcript values(v_id,v_code,Semes,s_year,NULL);
				SET output =1;
			ELSE 
				SET output =2;
			END IF;
		ELSE 
			SET output =3;
		END IF;
	END IF;
COMMIT;

END/
delimiter ;

#update uosoffering SET Enrollment=370 WHERE Semester='Q2' And year=2014 and UosCode='INFO2005';
#call course_enroll('INFO3220',5123,'Q1',2018,@out);
#call course_enroll( 'INFO1103',3213, 'Q1', 2018, @out);
#select @out;


         

         