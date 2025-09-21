#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <iomanip>
#include <limits>

using namespace std;
// Student structure to hold individual student records
class Student {
  public:
    int id;
    string name;
    int age;
    string major;
    string email;
};
// Global vector to store student records
vector<Student> students;
char input;

// helper to add a Student to the global vector (separated for reuse/testing)
void addStudent(const Student &stu) {
  students.push_back(stu);
}
// Function prototypes
void addStudentRecord() {
   Student student;
   cout << "Enter student ID: ";
   cin >> student.id;
  cin.ignore(numeric_limits<streamsize>::max(), '\n');
  cout << "Enter student name: ";
  getline(cin, student.name);
   cout << "Enter student age: ";
   cin >> student.age;
  cin.ignore(numeric_limits<streamsize>::max(), '\n');
  cout << "Enter student major: ";
  getline(cin, student.major);
  cout << "Enter student email: ";
  getline(cin, student.email);

  addStudent(student);
}
// Function to delete a student record
void deleteStudentRecord() {
    if (students.empty()) {
        cout << "No records to delete." << endl;
        return;
    }
    int id;
    cout << "Enter ID to delete: ";
    cin >> id;
    for (auto it = students.begin(); it != students.end(); ++it) {
        if (it->id == id) {
            students.erase(it);
            cout << "Record deleted." << endl;
            return;
        }
    }
    cout << "Record with ID " << id << " not found." << endl;
}
// Function to view all student records
void viewAllRecords() {
    if (students.empty()) {
        cout << "No student records." << endl;
        return;
    }
  // print header
  cout << left << setw(6) << "ID" << setw(20) << "Name" << setw(6) << "Age" << setw(12) << "Major" << "Email" << endl;
  cout << string(60, '-') << endl;
  for (const auto &st : students) {
    cout << left << setw(6) << st.id
       << setw(20) << st.name
       << setw(6) << st.age
       << setw(12) << st.major
       << st.email << endl;
  }
}
// Function to search a student record by ID
void searchRecordById() {
    if (students.empty()) {
        cout << "No student records." << endl;
        return;
    }
    int id;
    cout << "Enter ID to search: ";
    cin >> id;
    for (const auto &st : students) {
        if (st.id == id) {
            cout << "Found - ID: " << st.id << " Name: " << st.name << " Age: " << st.age
                 << " Major: " << st.major << " Email: " << st.email << endl;
            return;
        }
    }
    cout << "No record found with ID " << id << endl;
}
// Function to search student records by Name
void searchRecordByName() {
    if (students.empty()) {
        cout << "No student records." << endl;
        return;
    }
    string name;
    cout << "Enter name to search: ";
  cin.ignore(numeric_limits<streamsize>::max(), '\n');
  getline(cin, name);
    bool found = false;
    for (const auto &st : students) {
        if (st.name == name) {
            cout << "Found - ID: " << st.id << " Name: " << st.name << " Age: " << st.age
                 << " Major: " << st.major << " Email: " << st.email << endl;
            found = true;
        }
    }
    if (!found) cout << "No records found for name '" << name << "'." << endl;
}
// Function to count students by major
void countStudentByMajor() {
    if (students.empty()) {
        cout << "No student records." << endl;
        return;
    }
  unordered_map<string,int> counts;
  for (const auto &st : students) counts[st.major]++;
  cout << "Students by major:" << endl;
  for (const auto &p : counts) {
    cout << "  " << p.first << ": " << p.second << endl;
  }
}

// Main function loop
int main() {

  do {
    cout << "Please make your selection:\n"
       << "1. Add Student Record\n"
       << "2. Delete Student Record\n"
       << "3. View All Records\n"
       << "4. Search Record (by ID)\n"
       << "5. Search Record (by Name)\n"
       << "6. Count Students by Major\n"
       << "7. Exit" << endl;
    cin >> input;

    switch (input) {
      case '1':
        addStudentRecord();
        break;
      case '2':
        deleteStudentRecord();
        break;
      case '3':
        viewAllRecords();
        break;
      case '4':
        searchRecordById();
        break;
      case '5':
        searchRecordByName();
        break;
      case '6':
        countStudentByMajor();
        break;
      case '7':
        cout << "Exiting the program." << endl;
        break;
      default:
        cout << "Invalid selection. Please try again." << endl;
    }

  } while (input != '7');

  return 0;

} //end of main


