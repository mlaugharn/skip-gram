#include <cstdlib>
#include <cstring>
#include <iostream>
#include <fstream>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

void handle_err(const char* err_msg) {
	cerr << "\nerror: " << err_msg << endl;
	exit(1);
}

// recursive skip gram helper function
// this only calculates the offset (i.e. everything _after_ word_0)
// for the first word / word_0
void offset_walk(int i, int k, int n, int len, vector<int> sequence, vector<vector<int> >* offset_grams) {
	// recursive base case
	if (n == 0) {
		(*offset_grams).push_back(sequence);
		return;
	}
	// stop if this n-gram would run past the end of the corpus 
	if (i + n >= len) return;
	for (int skip_dist = 0; skip_dist < k + 1; skip_dist++) {
		int new_index = i + 1 + skip_dist;
		vector<int> sequence_cp = sequence;
		if (new_index < len) {
			sequence_cp.push_back(new_index);
			offset_walk(new_index, k, n - 1, len, sequence_cp, offset_grams);
		}
	}
}

// calculate and store offsets
// we use i=0 so that all offsets are relative to 0/portable
// note that we make (n-1)-grams because first word is pre-pended
void generate_offsets(vector<string>* corpus, int k, int n, vector<vector<int> >* offset_grams) {
	vector<int> sequence;
	size_t len = corpus->size();	
	// each sequence will end up being exactly n - 1 ints, might as well reserve that beforehand
	// each starts as a vector of n - 1 zeroes
	sequence.reserve(n - 1);
	
	// TODO
	// (*offset_grams).reserve(num_grams);

	offset_walk(0, k, n - 1, len, sequence, offset_grams);
}

//debugging stuff
void print_offsets(vector<vector<int> >* offset_grams) {
	for (vector<vector<int> >::iterator o_it = offset_grams->begin(); o_it != offset_grams->end(); ++o_it) {
		cout << "0 ";
		for (vector<int>::iterator oo_it = o_it->begin(); oo_it != o_it->end(); ++oo_it) {
			cout << *oo_it << " ";
		}
		cout << endl;
	}
}

// performs actual skip gram generation
// let corpus be a split corpus
// grams be an empty vec<vec<str> >
void generate_skip_grams(vector<string>* corpus, int k, int n, vector<vector<int> > *offset_grams, vector<vector<string> >* skip_grams) {
	size_t len = (*corpus).size();

	// look up word-grams from index-offsets
	// for word in corpus..
	for (size_t index = 0; index < len; index++) {
		// for offset-gram in offset-grams..
		for (vector<vector<int> >::iterator grams_it = offset_grams->begin(); grams_it != offset_grams->end(); ++grams_it) {
			vector<int> offsets = *grams_it;
			
			// initialize gram to just [word_in_corpus]
			vector<string> gram;
			gram.push_back((*corpus).at(index));
			
			for (vector<int>::iterator skip_it = offsets.begin(); skip_it != offsets.end(); ++skip_it) {
				int offset = *skip_it;
				int skipped_index = index + offset;
			
				// discard n-grams that will go over length of corpus
				if (skipped_index >= len) goto discard;

				string new_word = (*corpus).at(skipped_index);
				gram.push_back(new_word);
			}
			(*skip_grams).push_back(gram);
discard:
			gram.clear();
		}
	}
}

void print_skip_grams(vector<vector<string> >* skip_grams) {
	for (vector<vector<string > >::iterator skip_grams_it = skip_grams->begin(); skip_grams_it != skip_grams->end(); ++skip_grams_it) {
		for (vector<string>::iterator skip_gram_it = skip_grams_it->begin(); skip_gram_it != skip_grams_it->end(); ++skip_gram_it) {
			cout << *skip_gram_it << " ";
		}
		cout << endl;
	}
}

void read_words(const char* filename, size_t* word_count, vector<string>* corpus) {
	// iterate through the file
	ifstream textfile(filename, ifstream::in);
	stringstream ss;
	string line;
	size_t wc = 0;
	if (textfile.is_open()) {
		while (getline(textfile, line)) {
			stringstream ss(line);
			while (ss) {
				string word;
				ss >> word;
				if (word.length() > 0) {
					corpus->push_back(word);
					wc++;
				}
			}
		}
		*word_count = wc;
	} else handle_err("couldn't open file");
}

int main(int argc, char** argv) {
	size_t word_count;
	vector<string> corpus;
	vector<vector<int> > offset_grams;

	// handling arguments
	if (argc < 4) {
		handle_err("usage: ./s-g filename k n [--skip-grams-only]");
	}

	const char* filename = argv[1];
	int k = atoi(argv[2]); 	
	int n = atoi(argv[3]);
	bool verbose = !(argc > 4 && strcmp("--skip-grams-only", argv[4]) == 0); // print only skip-grams

	// k must be non-negative.. skipping backwards is meaningless
	if (k < 0) handle_err("k < 0"); 

	// must be > 1, otherwise n-grams contain no associations
	if (n < 2) handle_err("n < 2");

	// read words from input file
	if (verbose) cout << "Reading words... ";
	read_words(filename, &word_count, &corpus);
	if (verbose) cout << "Finished reading " << word_count << " words." << endl;

	// generate indices of n-grams relative to word_0
	generate_offsets(&corpus, k, n, &offset_grams);
	if (verbose) print_offsets(&offset_grams);
	if (verbose) cout << offset_grams.size() << " " << n - 1 << "-offset-grams generated." << endl;

	// create the skip grams!
	vector<vector<string> > grams;
	generate_skip_grams(&corpus, k, n, &offset_grams, &grams);
	if (verbose) cout << grams.size() << " skip grams generated." << endl;
	if (!verbose) print_skip_grams(&grams);
	exit(0);
}
