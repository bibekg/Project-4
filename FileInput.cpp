#include <iostream> // needed for any I/O
#include <fstream>  // needed in addition to <iostream> for file I/O
#include <sstream>  // needed in addition to <iostream> for string stream I/O
#include <string>
#include <unordered_map>
using namespace std;

int main()
{
		  // Open the file for input
    ifstream inf("expenses");
		  // Test for failure to open
    if ( ! inf)
    {
        cout << "Cannot open expenses file!" << endl;
        return 1;
    }
    
		  // This will hold the expense totals
    unordered_map<string, double> expenses;
    
		  // Read each line.  The return value of getline is treated
		  // as true if a line was read, false otherwise (e.g., because
		  // the end of the file was reached).
    string line;
    while (getline(inf, line))
    {
        // To extract the information from the line, we'll
        // create an input stringstream from it, which acts
        // like a source of input for operator>>
        istringstream iss(line);
        string category;
        double amt;
        // The return value of operator>> acts like false
        // if we can't extract a word followed by a number
        if ( ! (iss >> category >> amt) )
        {
            cout << "Ignoring badly-formatted input line: " << line << endl;
            continue;
        }
        // If we want to be sure there are no other non-whitespace
        // characters on the line, we can try to continue reading
        // from the stringstream; if it succeeds, extra stuff
        // is after the double.
        char dummy;
        if (iss >> dummy) // succeeds if there a non-whitespace char
            cout << "Ignoring extra data in line: " << line << endl;
        
        // Add data to expenses map
        expenses[category] += amt;
    }
    
		  // Print the totals
    cout.setf(ios::fixed);
    cout.precision(2);
    for (unordered_map<string, double>::iterator p = expenses.begin();
         p != expenses.end(); p++)
        cout << p->first << ": $" << p->second << endl;
}
