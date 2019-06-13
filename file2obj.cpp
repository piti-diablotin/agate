#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include <sstream>

using namespace std;

int main(int argc, char** argv) {

  for (int arg = 1; arg < argc; ++arg) {
    string filename(argv[arg]);
    ifstream file(filename,ios::in|ios::binary);
    if (!file) {
      cerr << filename << " in not readable" << endl;
      continue;
    }
    file.seekg(0,file.end);
    int length = file.tellg();
    file.seekg(0,file.beg);
    clog << filename << " is " << length << " bytes" << endl;
    unsigned char *buffer = new unsigned char[length];
    file.read((char*)buffer,length);
    file.close();
    stringstream output;
    output << "const unsigned char file[] = {" << endl << "  ";
    output << hex;
    for (int c = 0; c < length ; ++c ) {
      output << "0x" << setfill('0') << setw(sizeof(char)*2) << (unsigned short)buffer[c] << ", ";
      if ( c%16 == 15 && c != length-1) {
        output << endl << "  ";
      }
    }
    output << dec;
    output.seekp(-2,output.end);
    output << endl << "};" << endl;
    output << "std::ofstream ofile(\"ref_" << filename << "\",std::ios::out|std::ios::binary);" << endl;
    output << "ofile.write((char*)file," << length <<");" << endl;
    output << "ofile.close();" << endl;
    cout << output.str();
  }
  return 0;
}
