/**
 * File: state.h
 * Author: G. Guidi, E. Younis
 * Description: Xavier State Type Header.
 */

#include "state.h"

namespace xavier
{
	State State::initState(Seed& _seed, std::string const& hseq, std::string const& vseq,
				ScoringScheme& scoringScheme, int const &_scoreDropOff)
	{
		State state;
		state.seed = _seed;

		state.hlength = hseq.length() + 1;
		state.vlength = vseq.length() + 1;

		if (state.hlength < VECTORWIDTH || state.vlength < VECTORWIDTH)
		{
			state.seed.setEndH(state.hlength);
			state.seed.setEndV(state.vlength);
		}

		state.queryh = new int8_t[state.hlength + VECTORWIDTH];
		state.queryv = new int8_t[state.vlength + VECTORWIDTH];

		std::copy(hseq.begin(), hseq.begin() + hlength, state.queryh);
		std::copy(vseq.begin(), vseq.begin() + vlength, state.queryv);

		std::fill(state.queryh + state.hlength, state.queryh + state.hlength + VECTORWIDTH, NINF);
		std::fill(state.queryv + state.vlength, state.queryv + state.vlength + VECTORWIDTH, NINF);

		state.matchScore    = scoringScheme.getMatchScore();
		state.mismatchScore = scoringScheme.getMismatchScore();
		state.gapScore      = scoringScheme.getGapScore();

		state.vzeros = _mm256_setzero_si256();
		state.vgapScore.set (state.gapScore);
		state.vmatchScore.set (state.matchScore);
		state.vmismatchScore.set (state.mismatchScore);

		state.hoffset = LOGICALWIDTH;
		state.voffset = LOGICALWIDTH;

		state.bestScore    = 0;
		state.currScore    = 0;
		state.scoreOffset  = 0;
		state.scoreDropOff = _scoreDropOff;
		state.xDropCond    = false;

		return state;
	}

	State::~State()
	{
		delete [] queryh;
		delete [] queryv;
	}

	int State::getScoreOffset ()
	{
		return scoreOffset;
	}

	int State::getBestScore ()
	{
		return bestScore;
	}

	int State::getCurrScore ()
	{
		return currScore;
	}

	int State::getScoreDropoff ()
	{
		return scoreDropOff;
	}

	void State::setScoreOffset (int _scoreOffset)
	{
		scoreOffset = _scoreOffset;
	}

	void State::setBestScore (int _bestScore)
	{
		bestScore = _bestScore;
	}

	void State::setCurrScore (int _currScore)
	{
		currScore = _currScore;
	}

	int8_t State::getMatchScore ()
	{
		return matchScore;
	}
	int8_t State::getMismatchScore ()
	{
		return mismatchScore;
	}
	int8_t State::getGapScore ()
	{
		return gapScore;
	}

	VectorRegister State::getQueryH ()
	{
		return vqueryh;
	}

	VectorRegister State::getQueryV ()
	{
		return vqueryv;
	}

	VectorRegister State::getAntiDiag1 ()
	{
		return antiDiag1;
	}

	VectorRegister State::getAntiDiag2 ()
	{
		return antiDiag2;
	}

	VectorRegister State::getAntiDiag3 ()
	{
		return antiDiag3;
	}

	VectorRegister State::getVmatchScore ()
	{
		return vmatchScore;
	}

	VectorRegister State::getVmismatchScore ()
	{
		return vmismatchScore;
	}

	VectorRegister State::getVgapScore ()
	{
		return vgapScore;
	}

	VectorRegister State::getVzeros ()
	{
		return vzeros;
	}

	void State::updateQueryH (uint8_t idx, int8_t value)
	{
		vqueryh.internal.elems[idx] = value;
	}

	void State::updateQueryV (uint8_t idx, int8_t value)
	{
		vqueryv.internal.elems[idx] = value;
	}

	void State::updateAntiDiag1 (uint8_t idx, int8_t value)
	{
		antiDiag1.internal.elems[idx] = value;
	}

	void State::updateAntiDiag2 (uint8_t idx, int8_t value)
	{
		antiDiag2.internal.elems[idx] = value;
	}

	void State::updateAntiDiag3 (uint8_t idx, int8_t value)
	{
		antiDiag3.internal.elems[idx] = value;
	}

	void State::broadcastAntiDiag1 (int8_t value)
	{
		antiDiag1.set (value);
	}

	void State::broadcastAntiDiag2 (int8_t value)
	{
		antiDiag2.set (value);
	}

	void State::broadcastAntiDiag3 (int8_t value)
	{
		antiDiag3.set (value);
	}

	void State::setAntiDiag1 (VectorRegister vector)
	{
		antiDiag1 = vector;
	}

	void State::setAntiDiag2 (VectorRegister vector)
	{
		antiDiag2 = vector;
	}

	void State::setAntiDiag3 (VectorRegister vector)
	{
		antiDiag3 = vector;
	}

	void State::moveRight ()
	{
		/* (a) shift to the left on query horizontal */
		vqueryh.lshift();
		vqueryh.internal.elems[LOGICALWIDTH - 1] = queryh[hoffset++];

		/* (b) shift left on updated vector 1: this places the right-aligned vector 2 as a left-aligned vector 1 */
		antiDiag1 = antiDiag2;
		antiDiag2 = antiDiag3;
		antiDiag1.lshift ();
	}

	void State::moveDown ()
	{
		/* (a) shift to the right on query vertical */
		vqueryv.rshift ();
		vqueryv.internal.elems[0] = queryv[voffset++];

		/* (b) shift to the right on updated vector 2: this places the left-aligned vector 3 as a right-aligned vector 2 */
		antiDiag1 = antiDiag2;
		antiDiag2 = antiDiag3;
		antiDiag2.rshift ();
	}

	void operator+=(State& state1, const State& state2)
	{
		state1.bestScore = state1.bestScore + state2.bestScore;
		state1.currScore = state1.currScore + state2.currScore;
	}
}