#define _CRT_SECURE_NO_WARNINGS
#include <mysql.h>
#include <string>
#include <ctime>
#include <iostream>
#include <iomanip>
#include <vector>
using namespace std;

class StudentSystem {
private:
    MYSQL * conn;
    MYSQL_RES* res_set;
    MYSQL_ROW row;
    
public:
    StudentSystem() {
        conn = mysql_init(NULL);
        mysql_real_connect(conn, "localhost", "root", "Xs199518", "project3-nudb", 3306, 0, CLIENT_MULTI_STATEMENTS);
    }
    
    ~StudentSystem() {
        mysql_close(conn);
    }
    
    bool login(string id, string password) {
        string query = "SELECT password FROM Student WHERE id = " + id + ";";
        mysql_query(conn, query.c_str());
        res_set = mysql_store_result(conn);
        if (!(row = mysql_fetch_row(res_set)))
        {
            return false;
        }
        mysql_free_result(res_set);
        return string(row[0])==password;
    }
    
    void menu(string id) {
        while (1) {
            cout <<endl<< "/***********MAIN MENU**************/" << endl;
            string year, sem;
            time_t now = time(0);
            tm *ltm = localtime(&now);
            int month = ltm->tm_mon + 1;
            if (month >= 9 && month <= 12)
                sem = "Q1";
            else if (month >= 1 && month <= 3)
                sem = "Q2";
            else if (month >= 4 && month <= 6)
                sem = "Q3";
            else
                sem = "Q4";
            year = to_string(ltm->tm_year + 1900);
            string query = "SELECT unitofstudy.UoSCode, unitofstudy.UoSName "
            "FROM transcript "
            "JOIN unitofstudy ON transcript.UoSCode = unitofstudy.UosCode "
            "WHERE StudId = " + id + " AND YEAR=" + year + " AND SEMESTER=\'" + sem + "\';";
            if (mysql_query(conn, query.c_str()))
                fprintf(stderr, "%s\n", mysql_error(conn));
            res_set = mysql_store_result(conn);
            while (row = mysql_fetch_row(res_set)) {
                cout << (row[0] ? row[0] : "NULL") << "\t" << (row[1] ? row[1] : "NULL") << endl;
            }
            mysql_free_result(res_set);
            cout << "1: Transcript" << endl;
            cout << "2: Enroll" << endl;
            cout << "3: Withdraw" << endl;
            cout << "4: Personal Details" << endl;
            cout << "5: Logout" << endl;
            char index; cin >> index;
            switch (index) {
                case '1':
                    transcript(id); break;
                case '2':
                    enroll(id); break;
                case '3':
                    withdraw(id); break;
                case '4':
                    personalInfo(id); break;
                case '5':
                    return; break;
                default:
                    cout << "Please choose an correct option!" << endl;break;
            }
            
        }
    }
    
    void enroll(string id) {
        cout << "/***********Enrollment**************/" << endl;
        string thisYear, thisSem, nextYear, nextSem;
        time_t now = time(0);
        tm *ltm = localtime(&now);
        int month = ltm->tm_mon + 1;
        thisYear=to_string(ltm->tm_year + 1900);
        nextYear = thisYear;
        if (month >= 9 && month <= 12) {
            thisSem = "Q1";
            nextSem = "Q2";
            nextYear = to_string(ltm->tm_year + 1900 + 1);
        }
        else if (month >= 1 && month <= 3) {
            thisSem = "Q2";
            nextSem = "Q3";
        }
        else if (month >= 4 && month <= 6) {
            thisSem = "Q3";
            nextSem = "Q4";
        }
        else {
            thisSem = "Q4";
            nextSem = "Q1";
        }
        string query = "DROP VIEW IF EXISTS offering;";
        if (mysql_query(conn, query.c_str()))
            fprintf(stderr, "%s\n", mysql_error(conn));
        query = "CREATE VIEW offering(UosCode, UosName, Year, Semester) AS "
        "SELECT unitofstudy.UoSCode, unitofstudy.UoSName, Year, Semester "
        "FROM uosoffering "
        "JOIN unitofstudy using(UosCode) "
        "WHERE (YEAR=" + thisYear + " AND SEMESTER=\'" + thisSem + "\') OR (YEAR="+nextYear+" AND SEMESTER =\'"+nextSem+"\') "
        "ORDER BY Semester;";
        if (mysql_query(conn, query.c_str()))
            fprintf(stderr, "%s\n", mysql_error(conn));
        query = "SELECT * FROM offering";
        if (mysql_query(conn, query.c_str()))
            fprintf(stderr, "%s\n", mysql_error(conn));
        res_set = mysql_store_result(conn);
        cout << setiosflags(ios::left);
        cout << "index " << setw(15) << "Course Code" << "\t" << setw(40)<< "Course Name" << "\t" << "Year" << "\t" << "Semester" << endl;
        vector<vector<string> > offering_table( (int)mysql_num_rows(res_set), vector<string>(4));
        int count = 0;
        while (row = mysql_fetch_row(res_set)) {
            for (int i = 0; i < 4; i++) {
                offering_table[count][i] = string(row[i] ? row[i] : "NULL");
            }
            count++;
        }
        for(int i=0;i<offering_table.size();i++)
            cout << setw(4) << i+1 << ": " << setw(15) << offering_table[i][0] << "\t" << setw(40) << offering_table[i][1] << "\t" << offering_table[i][2] << "\t" << offering_table[i][3] << endl;
        mysql_free_result(res_set);
        
        while (1) {
            cout << "Enter the index of the course you would like to enroll or 0 to return to Main Menu:"<<endl ;
            int in;
            string strNum; cin >> strNum;
            if (strNum.find_first_not_of("0123456789") != string::npos) {
                cout << "Please enter a number!" << endl;
                continue;
            }
            in = stoi(strNum);
            if (in == 0)
                return;
            else if (in > offering_table.size() || in < 0) {
                cout << "Please enter a number between 0 and " << offering_table.size() << endl;
            }
            else{
                query = "call course_enroll ('" + offering_table[in-1][0] + "'," + id + ", '"+ offering_table[in - 1][3] +"',"+ offering_table[in - 1][2] +",@out);";
                if (mysql_query(conn, query.c_str()))
                    fprintf(stderr, "%s\n", mysql_error(conn));
                mysql_query(conn, "SELECT @out;");
                res_set = mysql_store_result(conn);
                row = mysql_fetch_row(res_set);
                mysql_free_result(res_set);
                switch (*row[0]) {
                    case '0':
                        cout << "You have already enrolled this course." << endl; break;
                    case '1':
                        cout << "You have enrolled this course successfully." << endl; break;
                    case '2':
                        cout << "This class is full." << endl; break;
                    case '3':
                        cout << "You are not satisfied with the requirement of this course." << endl; break;
                    default:
                        cout << "Error!" << endl; break;
                }
                cout << endl;
            }
        }
    }
    
    void transcript(string id) {
        cout << "/***********TRANSCRIPT**************/"<<endl;
        string query = "DROP VIEW IF EXISTS details;";
        if (mysql_query(conn, query.c_str()))
            fprintf(stderr, "%s\n", mysql_error(conn));
        query = "CREATE VIEW details(UosCode, UoSName, Year, Semester, Enrollment, MaxEnrollment, lecturer, Grade) AS "
        "SELECT unitofstudy.UoSCode, unitofstudy.UoSName, transcript.Year, transcript.Semester, uosoffering.Enrollment, uosoffering.MaxEnrollment, faculty.Name, transcript.Grade "
        "FROM transcript "
        "JOIN unitofstudy ON transcript.UoSCode = unitofstudy.UoSCode "
        "JOIN uosoffering ON transcript.UoSCode = uosoffering.UoSCode AND transcript.Year = uosoffering.Year AND transcript.Semester = uosoffering.Semester "
        "JOIN faculty ON uosoffering.InstructorId = faculty.ID "
        "WHERE StudId = " + id + ";";
        if (mysql_query(conn, query.c_str()))
            fprintf(stderr, "%s\n", mysql_error(conn));
        
        query="SELECT * FROM details";
        if(mysql_query(conn, query.c_str()))
            fprintf(stderr, "%s\n", mysql_error(conn));
        res_set = mysql_store_result(conn);
        cout<<setiosflags(ios::left);
        cout << "index " << setw(50) << "Course Name"<< "\t" << "Grade"<< endl;
        vector<vector<string> > transcript_table((int)mysql_num_rows(res_set), vector<string>(8));
        int count = 0;
        while (row = mysql_fetch_row(res_set)) {
            for (int i = 0; i < 8; i++) {
                transcript_table[count][i] = string(row[i] ? row[i] : "NULL");
            }
            count++;
        }
        for(int i=0;i<transcript_table.size();i++){
            cout <<setw(4) << i+1<<": " << setw(50) << transcript_table[i][1] << "\t" << transcript_table[i][7] << endl;
        }
        mysql_free_result(res_set);
        while (1) {
            cout << "Enter the index of course to see details or 0 to return to main menu" << endl;
            int index;
            string strNum; cin >> strNum;
            if (strNum.find_first_not_of("0123456789") != string::npos) {
                cout << "Please enter a number!" << endl;
                continue;
            }
            index = stoi(strNum)-1;
            if (index == -1)
                return;
            else if (index > transcript_table.size() - 1 || index < -1) {
                cout << "Please enter a number between 1 and " << transcript_table.size() << endl;
                continue;
            }
            if (index >= 0 && index < transcript_table.size()) {
                cout << setw(15) << "course number " << setw(45) << "course name " << setw(5) << "year " << setw(8) << "quarter " <<
                setw(13) << "#enrollment " << setw(15) << "max enrollment " << setw(30) << "lecturer " << setw(6) << "grade " << endl;
                cout << setw(15) << transcript_table[index][0] << setw(45) << transcript_table[index][1] << setw(5) << transcript_table[index][2] << setw(8) << transcript_table[index][3] <<
                setw(13) << transcript_table[index][4] << setw(15) << transcript_table[index][5] << setw(30) << transcript_table[index][6] << setw(6) << transcript_table[index][7] << endl;
            }
            else
                return;
        }
    }
    
    void withdraw(string id) {
        cout << "/***********WITHDRSW PAGE**************/" << endl;
        cout << "Enter the Code of the course you would like to withdraw: ";
        string c_code; cin >> c_code;
        string query="call course_withdraw ('"+c_code+"',"+id+",@out);";
        if (mysql_query(conn, query.c_str()))
            fprintf(stderr, "%s\n", mysql_error(conn));
        mysql_query(conn, "SELECT @out;");
        res_set = mysql_store_result(conn);
        row = mysql_fetch_row(res_set);
        mysql_free_result(res_set);
        switch (*row[0]) {
            case '0':
                cout << "You have not enrolled this course." << endl; break;
            case '1':
                cout << "You have withdrawed this course successfully." << endl; break;
            case '2':
                cout << "You cannot withdraw a course you've finished." << endl; break;
            default:
                cout << "Error!" << endl; break;
        }
        cout << endl;
        
        mysql_query(conn, "SELECT * FROM class_status;");
        res_set = mysql_store_result(conn);
        if (!(row = mysql_fetch_row(res_set)))
            cout << "empty row" << endl;
        mysql_free_result(res_set);
        if (row[0] && string(row[0]) == "1") {
            cout << "Warning: number of student is lower than a half!" << endl << endl;
        }
    }
    
    void personalInfo(string id) {
        cout << "/***********Personal Infomation*********************/" << endl;
        string query="SELECT * FROM student WHERE Id="+id+";";
        if (mysql_query(conn, query.c_str()))
            fprintf(stderr, "%s\n", mysql_error(conn));
        res_set = mysql_store_result(conn);
        row = mysql_fetch_row(res_set);
        mysql_free_result(res_set);
        cout << setiosflags(ios::left);
        cout << "Student ID: " << row[0] << endl <<
        "Name: " << row[1]<<endl<<
        "Password: " << row[2]<<endl<<
        "Address: " << row[3]<<endl;
        while (1) {
            cout << endl<<"Do you want to modify your infomation?" << endl;
            cout << "Enter p to change password or a to modified address. Press any other key to return Main Menu." << endl;
            char in; cin >> in;
            if (in == 'p' || in == 'P') {
                string pwd1, pwd2;
                cout << "Enter your new password:";
                cin >> pwd1;
                cout << "Enter your password again:";
                cin >> pwd2;
                if (pwd1 == pwd2) {
                    string query = "UPDATE student SET Password='" + pwd1 + "' WHERE Id=" + id + ";";
                    if (mysql_query(conn, query.c_str()))
                        fprintf(stderr, "%s\n", mysql_error(conn));
                    cout << "Password changed successfully." << endl;
                }
                else
                    cout << "The password you enter don't match!" << endl;
            }
            if (in == 'a' || in == 'A') {
                string addr;
                cout << "Enter your new address:";
                cin >> addr;
                string query = "UPDATE student SET Address='" + addr + "' WHERE Id=" + id + ";";
                if (mysql_query(conn, query.c_str()))
                    fprintf(stderr, "%s\n", mysql_error(conn));
                cout << "Address changed successfully." << endl;
            }
            else
                return;
        }
        
    }
};

void demo() {
    StudentSystem sys;
    while (1) {
        string username, password;
        cout << endl<<"Please enter your username and password." << endl << "Username:";
        cin >> username;
        cout << "Password:";
        cin >> password;
        if (!sys.login(username, password))
            cout << "The username and/or password you specified are not correct.";
        else {
            sys.menu(username);
        }
    }
}

int main(int argc, const char * argv[])
{
    demo();
    system("PAUSE");
}
