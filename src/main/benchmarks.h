/*
 * benchmarks.h
 *
 *  Created on: May 5, 2014
 *      Author: underscore
 */

#ifndef __BENCHMARKS_H__
#define __BENCHMARKS_H__

#include <string>
#include <vector>

using std::string;
using std::vector;


/**
 * Simple implementation for the word count map task.
 * The function receives:
 *  - key (current file pointer)
 *  - line (read from input file (it may include a '\n' at the end))
 *  - imap (where output data should be placed).
 *  - some bechmark specific data (wc doesn't need it).
 */
void wc_map(
		int k,
		string v,
		vector<std::map<string, vector<string> > >* imap,
		void* null) {
	string token;
	int red = 0;

	// remove trailing new line if exists.
	if(*(--v.end()) == '\n') { v.erase(--v.end()); }
	std::istringstream iss(v);
	// For every word in line, apply the modulo to the first character and
	// insert some place holder inside a map.
	while(getline(iss, token, ' '))
	{ (*imap)[*token.c_str() % imap->size()][token].push_back("1"); }
}

/**
 * Implementation for the word count map task.
 * The function receives:
 *  - key
 *  - values
 *  - omap (where output data is stored until it is written to file).
 *  - some bechmark specific data (wc doesn't need it).
 */
void wc_reduce(
		string k,
		vector<string> v,
		std::map<string, vector<string> >* omap,
		void* null) {
	std::stringstream ss;
	ss << v.size();
	(*omap)[k] = vector<string>(1, ss.str());
}

/**
 * Simple implementation for the grep map task.
 * The function receives:
 *  - key (current file pointer)
 *  - line (read from input file (it may include a '\n' at the end))
 *  - imap (where output data should be placed).
 *  - some bechmark specific data (needle to search for).
 */
void grep_map(
		int k,
		string v,
		vector<std::map<string, vector<string> > >* imap,
		void* needle) {
	if(v.find((char*)needle) != std::string::npos)
	{ (*imap)[k % imap->size()]["grep"].push_back(v); }
}

/**
 * TODO - doc.
 */
void grep_reduce(
		string k,
		vector<string> v,
		std::map<string, vector<string> >* omap,
		void* needle) {
	(*omap)["grep"].insert(
			v.end(), (*omap)["grep"].begin(), (*omap)["grep"].end());
}

/**
 *  TODO - doc.
 */
void kmeans_map(
		int k,
		string v,
		vector<std::map<string, vector<string> > >* imap,
		void* needle) {}

/**
 * TODO - doc.
 */
void kmeans_reduce(
		string k,
		vector<string> v,
		std::map<string, vector<string> >* omap,
		void* needle) {}

/**
 *  TODO - doc.
 */
void terasort_map(
		int k,
		string v,
		vector<std::map<string, vector<string> > >* imap,
		void* null) {}

/**
 * TODO - doc.
 */
void terasort_reduce(
		string k,
		vector<string> v,
		std::map<string, vector<string> >* omap,
		void* null) {}

/**
 *  TODO - doc.
 */
void sort(
		int k,
		string v,
		vector<std::map<string, vector<string> > >* imap,
		void* null) {	(*imap)[k % imap->size()][v].push_back("sort"); }

/**
 * TODO - doc.
 */
void sort(
		string k,
		vector<string> v,
		std::map<string, vector<string> >* omap,
		void* null) { (*omap)[k].push_back("sort"); }

#endif /* BENCHMARKS_H_ */
