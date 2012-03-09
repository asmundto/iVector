#include "iVectIO.h"
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>

using namespace std;

//typedef pair <int, double> I_D_PAIR;
typedef pair <string, int> S_I_PAIR;

//Language->Languageid
//const static S_I_PAIR LANGUAGES[] = {S_I_PAIR("ARABIC_EGYPT", 1), S_I_PAIR("ENG_GENRL", 2), S_I_PAIR("ENG_SOUTH", 3), S_I_PAIR("FARSI", 4),
//	S_I_PAIR("FRENCH_CAN", 5), S_I_PAIR("GERMAN", 6), S_I_PAIR("HINDI", 7), S_I_PAIR("JAPANESE", 8), S_I_PAIR("KOREAN", 9), S_I_PAIR("MANDARIN_M", 10), 
//	S_I_PAIR("MANDARIN_T", 11), S_I_PAIR("SPANISH", 12), S_I_PAIR("SPANISH_CAR", 13), S_I_PAIR("TAMIL", 14), S_I_PAIR("VIETNAMESE", 15) };
const static S_I_PAIR LANGUAGES[] = {S_I_PAIR("ENG_GENRL", 1), S_I_PAIR("SPANISH", 2), S_I_PAIR("MANDARIN_M", 3), S_I_PAIR("FARSI", 4),
	S_I_PAIR("FRENCH_CAN", 5), S_I_PAIR("GERMAN", 6), S_I_PAIR("HINDI", 7), S_I_PAIR("JAPANESE", 8), S_I_PAIR("KOREA", 9), S_I_PAIR("TAMIL", 10),
	S_I_PAIR("VIETNAMESE", 11), S_I_PAIR("ARABIC_EGYPT", 12), S_I_PAIR("ENG_SOUTH", 14), S_I_PAIR("SPANISH_CAR", 15), S_I_PAIR("MANDARIN_T", 16),
	S_I_PAIR("ENGLISH", 1), S_I_PAIR("MANDARIN", 3), S_I_PAIR("FRENCH", 5), S_I_PAIR("ARABIC", 12)};
const static int OUT_OF_SET_LANG_NUM = 13;
const static int LANG_LIST_LENGTH = 19;

const static string SET_INPUTFILE_NAMES[] = {"train_list.txt", "devtest_list.txt", "nist_list.txt"};


//Very basic string splitting (no regards to double spaces and such)
vector<string> splitString(string & s, char splitsign) {
	vector<string> svect;
	size_t startpos = 0;
	size_t endpos = s.find(splitsign);
	while (endpos != string::npos) {
		svect.push_back(s.substr(startpos, endpos-startpos));
		startpos = endpos+1;
		endpos = s.find(splitsign, startpos);
	}
	svect.push_back(s.substr(startpos));
	return svect;
}
int getLanguageClass(string & language) {
	for (int i = 0; i < LANG_LIST_LENGTH; i++) {
		if (LANGUAGES[i].first == language) {
			return LANGUAGES[i].second;
		}
	}
	return OUT_OF_SET_LANG_NUM;
}

Document readDocument(int languageClass, string & fullpath, int dim, int featureNameCol, int featureValueCol) {
	ifstream inFile;
	inFile.open(fullpath.c_str());
	if (!inFile.is_open()) {
		cerr << "Unable to read file " << fullpath;
		exit(1);
	}
	
	HASH_I_D featureMap;
	string line;

	while (getline(inFile, line)) {
		vector<string> splitLine = splitString(line, ' ');
		int feature = atoi(splitLine[featureNameCol].c_str());
		double value = atof(splitLine[featureValueCol].c_str());
		featureMap.insert(I_D_PAIR(feature, value));
	}
	inFile.close();
	Document doc(languageClass, featureMap, dim);
	return doc;
}

void fetchDocumentsFromFileList(vector<Document> & documents, string fullPath, string baseDir, int dim, int languageCol, int fileNameCol, int featureNameCol, int featureValueCol) {
	ifstream inFile;
	inFile.open(fullPath.c_str());
	if (!inFile.is_open()) {
		cerr << "Unable to read file " << fullPath;
		exit(1);
	}
	string line;
	while (getline(inFile, line)) {
		vector<string> splitLine = splitString(line, ' ');
		int languageClass = getLanguageClass(splitLine[languageCol]);
		string filePath = baseDir+splitLine[fileNameCol];
		documents.push_back(readDocument(languageClass, filePath, dim, featureNameCol, featureValueCol));
	}
	inFile.close();
}
vector<Document> fetchDocumentsFromFileList(int speechSet, string fileListDir, string baseDir, int dim, bool limitFeature) {
	vector<Document> documents;
	int featureValueCol = 1;
	if (limitFeature) {
		featureValueCol = 2;
	}
	fetchDocumentsFromFileList(documents, fileListDir+SET_INPUTFILE_NAMES[speechSet], baseDir, dim, 0, 1, 0, featureValueCol);	
	return documents;
}
void writeSpace(FeatureSpace & space, string fullPath) {
	ofstream outFile;
	outFile.open(fullPath.c_str());
	if (!outFile.is_open()) {
		cerr << "Unable to write to file " << fullPath;
		exit(1);
	}
	outFile << space.mVector(0);
	for (unsigned int i = 1; i < space.height; i++) {
		outFile << " " << space.mVector(i);
	}
	
	for (unsigned int i = 0; i < space.height; i++) {
		outFile << "\n" << space.tMatrix(i, 0);
		for (unsigned int j = 1; j < space.width; j++) {
			outFile << " " << space.tMatrix(i, j);
		}
	}
	outFile.close();
}
FeatureSpace readSpace(string fullPath) {
	ifstream inFile;
	inFile.open(fullPath.c_str());
	if (!inFile.is_open()) {
		cerr << "Unable to open file " << fullPath;
	}
	boost::numeric::ublas::matrix<double> tMatrix;
	string line;
	getline(inFile, line);
	vector<string> splitLine = splitString(line, ' ');
	boost::numeric::ublas::vector<double> mVector(splitLine.size());
	for (unsigned int i = 0; i < splitLine.size(); i++) {
		mVector(i) = atof(splitLine[i].c_str());
	}
	while (getline(inFile, line)) {
		splitLine = splitString(line, ' ');
		boost::numeric::ublas::vector<double> row(splitLine.size());
		for (unsigned int i = 0; i < splitLine.size(); i++) {
			row(i) = atof(splitLine[i].c_str());
		}
		tMatrix.resize(tMatrix.size1()+1, splitLine.size(), true);//Should be preallocated, but is currently not used anyways
		boost::numeric::ublas::matrix_row<boost::numeric::ublas::matrix<double> > (tMatrix, tMatrix.size1()-1) = row;
	}
	inFile.close();
	return FeatureSpace(tMatrix, mVector);
}


void writeDocument(ofstream & outFile, Document & document) {
	outFile << document.languageClass;
	for (unsigned int i = 0; i < document.iVector.size(); i++) {
		outFile << " " << (i+1) << ":" << document.iVector(i);
	}
	outFile << "\n";
}

void writeDocuments(vector<Document> & documents, string fullPath) {
	ofstream outFile;
	outFile.open(fullPath.c_str());
	if (!outFile.is_open()) {
		cerr << "Unable to write to file " << fullPath;
		exit(1);
	}
	for (unsigned int i = 0; i < documents.size(); i++) {
		writeDocument(outFile, documents[i]);
	}
	outFile.close();
}


/*
//Deprecated
void readDocuments(int languageClass, string dirpath, vector<Document> & documents, int dim) {
	
	//OS specific (windows or unix-based assumed)
	#ifdef _WIN32
	WIN32_FIND_DATA FindData;
	HANDLE hFind;
	string searchString = dirpath+TRANS_SEARCH_PATTERN;
	hFind = FindFirstFile( searchString.c_str(), &FindData );

	if (hFind == INVALID_HANDLE_VALUE) {
		cerr << "Error finding files in directory " << dirpath;
		exit(1);
	}
	do {
		string filePath = dirpath+FindData.cFileName;
		documents.push_back(readDocument(languageClass, filePath, dim));
	} while (FindNextFile(hFind, &FindData) > 0);
	if (GetLastError() != ERROR_NO_MORE_FILES) {
		cerr << "Error while reading directory " << dirpath;
	}
	#else
	DIR *dpdf;
	struct dirent *epdf;

	dpdf = opendir(dirpath.c_str());
	//dirpath.c_str()????
	if (dpdf != NULL) {
		while (epdf = readdir(dpdf)) {
			string filepath = dirpath+string(epdf->d_name);
			if (filepath.find(TRANS_EXTENSION) > 0) {
				documents.push_back(readDocument(languageClass, filePath, dim));
			}
		}
		closedir (dpdf);
	} else {
		cerr << "Error while reading directory " << dirpath;
		exit(1);
	}
	#endif
}

//Deprecated
vector<Document> fetchDocuments(int speechSet, string speechPath, int dim) {
	//string speechPath = "c:\\test\\?S\\docnum30\\?L\\";//?S=one of speech sets, ?L=language
	vector<Document> documents;
	size_t temp;
	temp=speechPath.find("?S");
	if (temp != string::npos) {
		speechPath = speechPath.replace(temp, 2, SPEECH_SETS[speechSet]);
	}
	string dirPath = speechPath;
	temp = speechPath.find("?L");
	for (int i = 0; i < NUMBER_OF_LANGUAGES; i++) {
		if (temp != string::npos) {
			dirPath = speechPath.replace(temp, 2, LANGUAGES[i].first);
		}
		readDocuments(LANGUAGES[i].second, dirPath, documents, dim);
	}
	return documents;
}
*/




