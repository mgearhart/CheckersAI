#include "StudentAI.h"
#include <random>


bool movesEqual(const Move& m1, const Move& m2)
{
	if (m1.seq.size() != m1.seq.size())
		return false;

	for (size_t i = 0; i < m1.seq.size(); ++i)
		if ( !(m1.seq[i] == m2.seq[i]) )
			return false;
	return true;
}


StudentAI::StudentAI(int col,int row,int p)
	:AI(col, row, p), root{new Node()}, stack(), ourFirstMove{true}
{
	board = Board(col,row,p);
	board.initializeGame();
	player = 2;
}

StudentAI::~StudentAI()
{
	delete root;
}


Move StudentAI::GetMove(Move move)
{
	if (move.seq.empty())
		player = 1;
	else
		board.makeMove(move, player == 1 ? 2 : 1);

	if (!ourFirstMove)
	{
		size_t induced = 0;
		for (; induced < root->children.size(); ++induced)
			if (movesEqual(root->children[induced]->move, move))
				break;

		std::swap(root->children[induced], root->children.back());
		Node* const oldRoot = root;
		root = root->children.back();
		oldRoot->children.pop_back();
		delete oldRoot;
	}
	ourFirstMove = false;

	//DECIDE or for however long
	for (int its = 0 ; its < 1000; ++its)
	{
		stack.push_back(root);
		unsigned curPlayer = player; //whose turn it is at the node we're at

		//SELECTION
		while (!stack.back()->isLeaf())
		{
			stack.push_back(stack.back()->selectChildUCT());
			board.makeMove(stack.back()->move, curPlayer);
			curPlayer = curPlayer == 1 ? 2 : 1;
		}

		int isWin = 0xDEADBEEF;
		unsigned stackBackPlayer = curPlayer; //save for backpropagation
		if ( !(isWin = board.isWin(curPlayer == 1 ? 2 : 1)) )
		{
			//EXPANSION
			for (const auto& moveVec : board.getAllPossibleMoves(curPlayer))
				for (const Move& move : moveVec)
					stack.back()->addChild(move);

			//pushing the first child is nicely predictably behaved
			stack.push_back(stack.back()->children.front());
			board.makeMove(stack.back()->move, curPlayer);
			curPlayer = curPlayer == 1 ? 2 : 1;
			stackBackPlayer = curPlayer; //save for backpropagation

			//SIMULATION
			//DECIDE you can build this as a hidden part of the tree, unhiding as you expand down
			unsigned plys = 0;
			for (; !(isWin = board.isWin(curPlayer == 1 ? 2 : 1)); ++plys)
			{
				//DECIDE what playout policy
				const auto& moves = board.getAllPossibleMoves(curPlayer);
				const auto& randomPieceMoves = moves[rand() % moves.size()];
				const auto& m = randomPieceMoves[rand() % randomPieceMoves.size()];

				board.makeMove(m, curPlayer);
				curPlayer = curPlayer == 1 ? 2 : 1;
			}
			for (unsigned i = 0; i < plys; ++i)
				board.Undo();
		}

		//BACKPROPAGATION
		while (stack.size())
		{
			//DECIDE currently draws are our losses
			++stack.back()->visits;
			if (isWin == 2 && stackBackPlayer == 1 ||
				isWin == 1 && stackBackPlayer == 2 ||
				isWin == -1 && stackBackPlayer == player)
				++stack.back()->parentWins;

			stack.pop_back();
			stackBackPlayer = stackBackPlayer == 1 ? 2 : 1;
			if (stack.size()) board.Undo();
		}
	}

	//DECIDE highest visits or highest winrate? text says visits, vids say winrate
	size_t bestNodeIndex = 0; //arbitrary
	for (size_t i = 0; i < root->children.size(); ++i)
		if (static_cast<double>(root->children[i]->parentWins) / root->children[i]->visits >
		    static_cast<double>(root->children[bestNodeIndex]->parentWins) / root->children[bestNodeIndex]->visits )
			bestNodeIndex = i;
	std::swap(root->children[bestNodeIndex], root->children.back());
	
	Node* const oldRoot = root;
	root = root->children.back();
	oldRoot->children.pop_back();
	delete oldRoot;

	const Move ret(root->move);
	board.makeMove(ret, player);
	return ret;
}
