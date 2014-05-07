/*
 * benchmarks.h
 *
 *  Created on: May 5, 2014
 *      Author: underscore
 */

#ifndef __BENCHMARKS_H__
#define __BENCHMARKS_H__

#include <stdlib.h>

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
		void* null = NULL) {
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
		std::map<string, vector<string> >* omap) {
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
 * Reduce task for grep job.
 * It receives:
 *  - key (some key, not really important for this example),
 *  - v (vector of lines that contain the needle),
 *  - ompa (output map).
 */
void grep_reduce(
		string k,
		vector<string> v,
		std::map<string, vector<string> >* omap)
	{ (*omap)["grep"].insert((*omap)["grep"].end(), v.begin(), v.end() ); }

/**
 *  TODO -> page ranking.
 */
void kmeans_map(
		int k,
		string v,
		vector<std::map<string, vector<string> > >* imap,
		void* needle = NULL) {}

/**
 * TODO - page ranking.
 */
void kmeans_reduce(
		string k,
		vector<string> v,
		std::map<string, vector<string> >* omap) {}

/**
 * This is a very very, very, simplified implementation of terasort. This only
 * works with numbers. The partitioner assumes that all keys are uniformly
 * spread in the input space. min_number is assumed to be 0 (zero).
 */
void terasort_map(
		int k,
		string v,
		vector<std::map<string, vector<string> > >* imap,
		void* max_number)
	{ (*imap)[atol(v.c_str()) / (*((int*)max_number) / imap->size())]
	         [v].push_back(v); }

/**
 * This is the reduce part of the very simplified implementation of terasort.
 */
void terasort_reduce(
		string k,
		vector<string> v,
		std::map<string, vector<string> >* omap)
	{ (*omap)[k].insert((*omap)[k].end(), v.begin(), v.end() ); }

/**
 * This is an almost empty map task. This benchmark is meant to measure the
 * speed of the system and not some application.
 */
void sort_map(
		int k,
		string v,
		vector<std::map<string, vector<string> > >* imap,
		void* null = NULL) { (*imap)[k % imap->size()][v].push_back(v); }

/**
 * This is the reduce implementation for the sort job. This benchmark is meant
 * to measure the speed of the system and not some application.
 */
void sort_reduce(
		string k,
		vector<string> v,
		std::map<string, vector<string> >* omap)
	{ (*omap)[k].insert((*omap)[k].end(), v.begin(), v.end() ); }

#endif /* BENCHMARKS_H_ */
