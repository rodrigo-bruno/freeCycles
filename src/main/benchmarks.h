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
 * TODO - doc
 * input -> key(int) v(id rank olink ... olink)
 * output -> key(id) v(olink ... olink), key(olink) v(#r), ..., key(olink) v(#r)
 */
void pr_map(
		int k,
		string v,
		vector<std::map<string, vector<string> > >* imap,
		void* null = NULL) {
	string token, key;
	int rank = 0, i = 0;
	vector<string> *pagev = NULL, *out_pagev = NULL;
	std::stringstream ss;

	// remove trailing new line if exists.
	if(*(--v.end()) == '\n') { v.erase(--v.end()); }
	std::istringstream iss(v);

	// get page id.
	getline(iss, key, '=');
	pagev = &((*imap)[*key.c_str() % imap->size()][key]);

	// get page rank
	getline(iss, token, ';');
	rank = atol(token.c_str());

	// get all outgoing links
	while(getline(iss, token, ';'))	{ pagev->push_back(token); }

	// calculate how much to give to each outgoing link
	ss << rank / pagev->size();

	// for each outgoing link, give a share of own rank
	for(i = 0; i < pagev->size(); i++) {
		token = pagev->at(i);
		// if item is not link, ignore
		if(token.c_str()[0] == '#') { continue; }
		// get outgoing link vector
		out_pagev = &((*imap)[*(token.c_str()) % imap->size()][token]);
		// note that i am modifying the vector is beeing iterated.
		out_pagev->insert(out_pagev->begin(),"#" + ss.str());
		out_pagev == pagev ? i++ : 0;
	}
}

/**
 * TODO
 * input -> key(id) v( (olink | #r1) ... (olink | #rn) )
 * output -> key(id) v(id rank olink ... olink)
 */
void pr_reduce(
		string k,
		vector<string> v,
		std::map<string, vector<string> >* omap) {
	int ratio_sum = 0;
	std::stringstream ss;
	vector<string>* pagev = &((*omap)[k]);

	// for every element in intermediate vector
	for (vector<string>::iterator vit = v.begin(); vit != v.end(); vit++) {
		// if element is marked as ratio
		if(vit->c_str()[0] == '#') { ratio_sum += atol(vit->c_str() + 1); }
		// if element is output link, just copy it to output
		else { pagev->push_back(*vit); }
	}

	// insert ratio and k at the beginning of the vector
	ss << ratio_sum;
	pagev->insert(pagev->begin(), ss.str());
	pagev->insert(pagev->begin(), k);
}

/**
 * This is a very very, very, simplified implementation of terasort. This only
 * works with numbers. The partitioner assumes that all keys are uniformly
 * spread in the input space. min_number is assumed to be 0 (zero).
 */
void terasort_map(
		int k,
		string v,
		vector<std::map<string, vector<string> > >* imap,
		void* max_number) {
	if(*(--v.end()) == '\n') { v.erase(--v.end()); }
	(*imap)[atol(v.c_str()) / (*((int*)max_number) / imap->size())]
	       [v].push_back(v);
	}

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
		void* null = NULL) {
	// remove trailing new line if exists.
	if(*(--v.end()) == '\n') { v.erase(--v.end()); }
	(*imap)[k % imap->size()][v].push_back(v);
}

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
