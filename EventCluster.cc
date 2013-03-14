/**
 * @file EventCluster.cc
 * Implementation of class EventCluster
 *
 * @author Yi Qiao
 */

#include "EventCluster.h"
#include "SomaticEvent.h"
#include <cmath>

using namespace SubcloneExplorer;

void EventCluster::addEvent(SomaticEvent *event, bool updateFraction) {
	// check if the event already is a member

	bool alreadyExist = false;

	for(size_t i=0; i<_members.size(); i++) {
		if(_members[i] == event) {
			alreadyExist = true;
			break;
		}
	}

	if(!alreadyExist) {
		double oldFraction = _cellFraction * _members.size();
		_members.push_back(event);
		if(updateFraction)
			_cellFraction = (oldFraction + event->frequency) / _members.size();
	}
}

std::vector<EventCluster *> EventCluster::clustering(const std::vector<SomaticEvent *>& events, double threshold) {
	std::vector<EventCluster *> clusters;

	if(threshold < 0 || threshold > 1)
		return clusters;

	for(size_t eventIdx = 0; eventIdx < events.size(); eventIdx++) {
		double minDiff = -1;
		size_t minClusterIdx = 0;

		SomaticEvent *currentEvent = events[eventIdx];

		for(size_t clusterIdx = 0; clusterIdx < clusters.size(); clusterIdx++) {
			EventCluster *currentCluster = clusters[clusterIdx];
			double diff = fabs(currentCluster->cellFraction() - currentEvent->frequency);
			
			if(minDiff == -1 || minDiff > diff) {
				minDiff = diff;
				minClusterIdx = clusterIdx;
			}
		}

		if(clusters.size() > 0 && minDiff <= threshold) {
			// add the current event to the cluster
			clusters[minClusterIdx]->addEvent(currentEvent);
		}
		else {
			// create new cluster to contain the currentEvent;
			EventCluster *newCluster = new EventCluster();
			newCluster->addEvent(currentEvent);
			clusters.push_back(newCluster);
		}
	}
	return clusters;
}

DBObjectID_vec EventCluster::allObjectsOfSubclone(sqlite3 *database, sqlite3_int64 subcloneID) {
	sqlite3_stmt* st;
	int rc;
	DBObjectID_vec res_vec;

	std::string queryStr = "SELECT id FROM " + getTableName() + " WHERE ofSubcloneID=?";

	rc = sqlite3_prepare_v2(database, queryStr.c_str(), -1, &st, 0);
	if(rc != SQLITE_OK) {
		sqlite3_finalize(st);
		return res_vec;
	}

	rc = sqlite3_bind_int64(st, 1, subcloneID);
	if(rc != SQLITE_OK) {
		sqlite3_finalize(st);
		return res_vec;
	}

	while(sqlite3_step(st) == SQLITE_ROW) {
		sqlite3_int64 newID;
		newID = sqlite3_column_int64(st, 0);
		res_vec.push_back(newID);
	}

	sqlite3_finalize(st);
	return(res_vec);
}

/**********************************/
/*  IMPLEMENTATION OF Archivable  */
/**********************************/

std::string EventCluster::getTableName() {
	return "Clusters";
}

std::string EventCluster::createTableStatementStr() {
	return ", fraction REAL NOT NULL, ofSubcloneID INTEGER NULL REFERENCES Subclone(id)";
}

std::string EventCluster::createObjectStatementStr() {
	return "INSERT INTO Clusters (fraction, ofSubcloneID) VALUES (?,?);";
}

std::string EventCluster::updateObjectStatementStr() {
	return "UPDATE Clusters SET fraction=?, ofSubcloneID=? WHERE id=?;";
}

std::string EventCluster::selectObjectColumnListStr() {
	return "fraction, ofSubcloneID";
}

int EventCluster::bindObjectToStatement(sqlite3_stmt *statement) {
	int bind_loc = 1;
	sqlite3_bind_double(statement, bind_loc++, _cellFraction);
	if(ofSubcloneID> 0) {
		sqlite3_bind_int64(statement, bind_loc++, ofSubcloneID);
	}
	else {
		sqlite3_bind_null(statement, bind_loc++);
	}

	return bind_loc;
}

void EventCluster::updateObjectFromStatement(sqlite3_stmt *statement) {
	int col_pos = 0;
	_cellFraction= sqlite3_column_double(statement, col_pos++);

	if(sqlite3_column_type(statement, col_pos) != SQLITE_NULL) {
		ofSubcloneID = sqlite3_column_int64(statement, col_pos++);
	}
	else {
		ofSubcloneID = 0;
	}
}
