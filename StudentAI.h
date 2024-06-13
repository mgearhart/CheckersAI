#ifndef STUDENTAI_H
#define STUDENTAI_H
#include "AI.h"
#include "Board.h"

#include <cmath>

#pragma once


//DECIDE textbook says this is theoretically best but people often try many 
const double C = sqrt(2);


bool movesEqual(const Move& m1, const Move& m2);


//TODO test DESTRUCTOR?
class Node
{
public:

	Move move;
	unsigned visits;
	unsigned parentWins;
	std::vector<Node*> children;

	Node()
		: move(), visits{0}, parentWins{0}, children()
	{	
	}

	Node(const Move& move)
		: move(move), visits{0}, parentWins{0}, children()
	{
	}

	bool isLeaf() const noexcept
	{
		return children.empty();
	}

	void addChild(const Move& move)
	{
		children.push_back(new Node(move));
	}

	//undefined iff child is unvisited
	double UCT(Node* child) const
	{
		return static_cast<double>(child->parentWins) / child->visits + C * sqrt(log(visits) / child->visits);
	}

	//undefined iff has no child
	Node* selectChildUCT() const
	{
		if (!children[0]->visits)
			return children[0];
		Node* bestSoFar = children[0];
		for (Node* child : children)
		{
			if (!child->visits)
				return child;
			if (UCT(child) > UCT(bestSoFar))
				bestSoFar = child;
		}
		return bestSoFar;
	}

	~Node()
	{
		for (Node* child : children)
			delete child;
	}
};


//The following part should be completed by students.
//Students can modify anything except the class name and exisiting functions and varibles.
class StudentAI :public AI
{
public:
    Board board;
	StudentAI(int col, int row, int p);
	virtual Move GetMove(Move board);


	Node* root;
	std::vector<Node*> stack;
	bool ourFirstMove;
	~StudentAI();
};

#endif //STUDENTAI_H
