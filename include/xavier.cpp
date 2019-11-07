/**
 * File: xavier.h
 * Author: G. Guidi, E. Younis
 * Description: Xavier Main Source.
 */

#include<vector>
#include<iostream>
#include<omp.h>
#include<algorithm>
#include<inttypes.h>
#include<assert.h>
#include<iterator>
#include<x86intrin.h>

#include "xavier.h"

namespace xavier {

	void beg (State& state)
    {
        // we need one more space for the off-grid values and one more space for antiDiag2
        int DPmatrix[LOGICALWIDTH + 2][LOGICALWIDTH + 2];

        // DPmatrix initialization
        DPmatrix[0][0] = 0;
        for (int i = 1; i < LOGICALWIDTH + 2; i++)
        {
            DPmatrix[0][i] = -i;
            DPmatrix[i][0] = -i;
        }

        // DPmax tracks maximum value in DPmatrix for x-drop condition
        int DPmax = 0;

        // DPmatrix population
        for (int i = 1; i < LOGICALWIDTH + 2; i++) {
            // GG: we only need the upper-left triangular matrix
            for (int j = 1; j <= LOGICALWIDTH + 2 - i; j++) {

                int oneF = DPmatrix[i-1][j-1];

                // Comparing bases
                if (state.queryh[i-1] == state.queryv[j-1])
                    oneF += state.getMatchScore();
                else
                    oneF += state.getMismatchScore();

                int twoF = std::max (DPmatrix[i-1][j], DPmatrix[i][j-1]);
                twoF += state.getGapScore();

                DPmatrix[i][j] = std::max (oneF, twoF);

                // Heuristic to keep track of the max in inital stage of the computation
                if (DPmatrix[i][j] > DPmax)
                    DPmax = DPmatrix[i][j];
            }
        }

        for (int i = 0; i < LOGICALWIDTH; ++i) {
            state.updateQueryH (i, state.queryh[i + 1]);
            state.updateQueryV (i, state.queryv[LOGICALWIDTH - i]);
        }

        state.updateQueryH (LOGICALWIDTH, NINF);
        state.updateQueryV (LOGICALWIDTH, NINF);

        int antiDiagMax = std::numeric_limits<int8_t>::min();

        // Load DPmatrix into antiDiag1 and antiDiag2 vector and find max elem at the end of the initial stage in antiDiag1
        for (int i = 1; i < LOGICALWIDTH + 1; ++i) {
            int value1 = DPmatrix[i][LOGICALWIDTH - i + 1];
            int value2 = DPmatrix[i + 1][LOGICALWIDTH - i + 1];

            state.updateAntiDiag1 (i - 1, value1);
            state.updateAntiDiag2 (i, value2);

            if(value1 > antiDiagMax)
                antiDiagMax = value1;
        }

        state.updateAntiDiag1 (LOGICALWIDTH, NINF);
        state.updateAntiDiag2 (0, NINF);
        state.broadcastAntiDiag3 (NINF);

        state.setBestScore (DPmax);
        state.setCurrScore(antiDiagMax);

        if (antiDiagMax < DPmax - state.getScoreOffset ()) {
            state.xDropCond = true;

            state.seed.setEndH (state.hoffset);
            state.seed.setEndV (state.voffset);

            return;
        }
        // antiDiag2 going right, first computation of antiDiag3 is going down
    }

	void mid (State& state);

	void end (State& state);

	void onedirection (State& state);

	std::pair<int, int> xavier (Seed& seed, Direction direction, std::string const& target,
		std::string const& query, ScoringScheme& scoringScheme, int const &scoreDropOff);

} /* namespace xavier */

// void
// XavierPhase2(XavierState& state)
// {
// 	msg("Phase2");
// 	while(state.hoffset < state.hlength && state.voffset < state.vlength)
// 	{
// 		// antiDiag1F (final)
// 		// NOTE: -1 for a match and 0 for a mismatch
// 		vectorType match = cmpeqOp(state.get_vqueryh(), state.get_vqueryv());
// 		match = blendvOp(state.get_vmismatchCost(), state.get_vmatchCost(), match);
// 		vectorType antiDiag1F = addOp(match, state.get_antiDiag1());

// 		// antiDiag2S (shift)
// 		vectorUnionType antiDiag2S = shiftLeft(state.get_antiDiag2());

// 		// antiDiag2M (pairwise max)
// 		vectorType antiDiag2M = maxOp(antiDiag2S.simd, state.get_antiDiag2());

// 		// antiDiag2F (final)
// 		vectorType antiDiag2F = addOp(antiDiag2M, state.get_vgapCost());

// 		// Compute antiDiag3
// 		state.set_antiDiag3(maxOp(antiDiag1F, antiDiag2F));

// 		// we need to have always antiDiag3 left-aligned
// 		state.update_antiDiag3(LOGICALWIDTH, NINF);

// 		// TODO: x-drop termination
// 		// Note: Don't need to check x-drop every time
// 		// Create custom max_element that also returns position to save computation
// 		int8_t antiDiagBest = *std::max_element(state.antiDiag3.elem, state.antiDiag3.elem + VECTORWIDTH);
// 		state.set_curr_score(antiDiagBest + state.get_score_offset());

// 		int64_t scoreThreshold = state.get_best_score() - state.get_score_dropoff();
// 		if (state.get_curr_score() < scoreThreshold)
// 		{
// 			state.xDropCond = true;

// 			setBeginPositionH(state.seed, 0);
// 			setBeginPositionV(state.seed, 0);

// 			setEndPositionH(state.seed, state.hoffset);
// 			setEndPositionV(state.seed, state.voffset);

// 			return; // GG: it's a void function and the values are saved in XavierState object
// 		}

// 		if (antiDiagBest > CUTOFF)
// 		{
// 			int8_t min = *std::min_element(state.antiDiag3.elem, state.antiDiag3.elem + LOGICALWIDTH);
// 			state.set_antiDiag2(subOp(state.get_antiDiag2(), setOp(min)));
// 			state.set_antiDiag3(subOp(state.get_antiDiag3(), setOp(min)));
// 			state.set_score_offset(state.get_score_offset() + min);
// 		}

// 		// Update best
// 		if (state.get_curr_score() > state.get_best_score())
// 			state.set_best_score(state.get_curr_score());

// 		// TODO: optimize this
// 		int maxpos, max = 0;
// 		for (int i = 0; i < VECTORWIDTH; ++i)
// 		{
// 			if (state.antiDiag3.elem[i] > max)
// 			{
// 				maxpos = i;
// 				max = state.antiDiag3.elem[i];
// 			}
// 		}

// 		setEndPositionH(state.seed, state.hoffset);
// 		setEndPositionV(state.seed, state.voffset);

// 		if (maxpos > MIDDLE)
// 			state.moveRight();
// 		else
// 			state.moveDown();
// 	}
// }

// void
// XavierPhase4(XavierState& state)
// {
// 	msg("Phase4");
// 	int dir = state.hoffset >= state.hlength ? goDOWN : goRIGHT;

// 	for (int i = 0; i < (LOGICALWIDTH - 3); i++)
// 	{
// 		// antiDiag1F (final)
// 		// NOTE: -1 for a match and 0 for a mismatch
// 		vectorType match = cmpeqOp(state.get_vqueryh(), state.get_vqueryv());
// 		match = blendvOp(state.get_vmismatchCost(), state.get_vmatchCost(), match);
// 		vectorType antiDiag1F = addOp(match, state.get_antiDiag1());

// 		// antiDiag2S (shift)
// 		vectorUnionType antiDiag2S = shiftLeft(state.get_antiDiag2());

// 		// antiDiag2M (pairwise max)
// 		vectorType antiDiag2M = maxOp(antiDiag2S.simd, state.get_antiDiag2());

// 		// antiDiag2F (final)
// 		vectorType antiDiag2F = addOp(antiDiag2M, state.get_vgapCost());

// 		// Compute antiDiag3
// 		state.set_antiDiag3(maxOp(antiDiag1F, antiDiag2F));

// 		// we need to have always antiDiag3 left-aligned
// 		state.update_antiDiag3(LOGICALWIDTH, NINF);

// 		// TODO: x-drop termination
// 		// note: Don't need to check x drop every time
// 		// Create custom max_element that also returns position to save computation
// 		int8_t antiDiagBest = *std::max_element(state.antiDiag3.elem, state.antiDiag3.elem + VECTORWIDTH);
// 		state.set_curr_score(antiDiagBest + state.get_score_offset());

// 		int64_t scoreThreshold = state.get_best_score() - state.get_score_dropoff();

// 		if (state.get_curr_score() < scoreThreshold)
// 		{
// 			state.xDropCond = true;
// 			return; // GG: it's a void function and the values are saved in XavierState object
// 		}

// 		if (antiDiagBest > CUTOFF)
// 		{
// 			int8_t min = *std::min_element(state.antiDiag3.elem, state.antiDiag3.elem + LOGICALWIDTH);
// 			state.set_antiDiag2(subOp(state.get_antiDiag2(), setOp(min)));
// 			state.set_antiDiag3(subOp(state.get_antiDiag3(), setOp(min)));
// 			state.set_score_offset(state.get_score_offset() + min);
// 		}

// 		// Update best
// 		if (state.get_curr_score() > state.get_best_score())
// 			state.set_best_score(state.get_curr_score());

// 		// antiDiag swap, offset updates, and new base load
// 		short nextDir = dir ^ 1;

// 		if (nextDir == goRIGHT)
// 			state.moveRight();
// 		else
// 			state.moveDown();

// 		// Update direction
// 		dir = nextDir;
// 	}
// }

// //======================================================================================
// // X-DROP ADAPTIVE BANDED ALIGNMENT
// //======================================================================================

// void
// XavierOneDirection (XavierState& state) {

// 	// PHASE 1 (initial values load using dynamic programming)
// 	XavierPhase1(state);
// 	if(state.xDropCond)  return;

// 	// PHASE 2 (core vectorized computation)
// 	XavierPhase2(state);
// 	if(state.xDropCond) return;

// 	// PHASE 3 (align on one edge)
// 	// GG: Phase3 removed to read to code easily (can be recovered from simd/ folder or older commits)

// 	// PHASE 4 (reaching end of sequences)
// 	XavierPhase4(state);
// 	if(state.xDropCond) return;
// }

// std::pair<int, int>
// XavierXDrop
// (
// 	SeedX& seed,
// 	ExtDirectionL direction,
// 	std::string const& target,
// 	std::string const& query,
// 	ScoringSchemeX& scoringScheme,
// 	int const &scoreDropOff
// )
// {
// 	// TODO: add check scoring scheme correctness/input parameters

// 	if (direction == XAVIER_EXTEND_LEFT)
// 	{
// 		SeedX _seed = seed; // need temporary datastruct

// 		std::string targetPrefix = target.substr (0, getEndPositionH(seed));	// from read start til start seed (seed included)
// 		std::string queryPrefix  = query.substr  (0, getEndPositionV(seed));	// from read start til start seed (seed included)
// 		std::reverse (targetPrefix.begin(), targetPrefix.end());
// 		std::reverse (queryPrefix.begin(),  queryPrefix.end());

// 		XavierState result (_seed, targetPrefix, queryPrefix, scoringScheme, scoreDropOff);

// 		if (targetPrefix.length() >= VECTORWIDTH || queryPrefix.length() >= VECTORWIDTH)
// 			XavierOneDirection (result);

// 		setBeginPositionH(seed, getEndPositionH(seed) - getEndPositionH(result.seed));
// 		setBeginPositionV(seed, getEndPositionV(seed) - getEndPositionV(result.seed));

// 		return std::make_pair(result.get_best_score(), result.get_curr_score());
// 	}
// 	else if (direction == XAVIER_EXTEND_RIGHT)
// 	{
// 		SeedX _seed = seed; // need temporary datastruct

// 		std::string targetSuffix = target.substr (getBeginPositionH(seed), target.length()); 	// from end seed until the end (seed included)
// 		std::string querySuffix  = query.substr  (getBeginPositionV(seed), query.length());		// from end seed until the end (seed included)

// 		XavierState result (_seed, targetSuffix, querySuffix, scoringScheme, scoreDropOff);

// 		if (targetSuffix.length() >= VECTORWIDTH || querySuffix.length() >= VECTORWIDTH)
// 			XavierOneDirection (result);

// 		setEndPositionH (seed, getBeginPositionH(seed) + getEndPositionH(result.seed));
// 		setEndPositionV (seed, getBeginPositionV(seed) + getEndPositionV(result.seed));

// 		return std::make_pair(result.get_best_score(), result.get_curr_score());
// 	}
// 	else
// 	{
// 		SeedX _seed1 = seed; // need temporary datastruct
// 		SeedX _seed2 = seed; // need temporary datastruct

// 		std::string targetPrefix = target.substr (0, getEndPositionH(seed));	// from read start til start seed (seed not included)
// 		std::string queryPrefix  = query.substr  (0, getEndPositionV(seed));	// from read start til start seed (seed not included)

// 		std::reverse (targetPrefix.begin(), targetPrefix.end());
// 		std::reverse (queryPrefix.begin(),  queryPrefix.end());

// 		XavierState result1(_seed1, targetPrefix, queryPrefix, scoringScheme, scoreDropOff);

// 		if (targetPrefix.length() < VECTORWIDTH || queryPrefix.length() < VECTORWIDTH)
// 		{
// 			setBeginPositionH (seed, getEndPositionH(seed) - targetPrefix.length());
// 			setBeginPositionV (seed, getEndPositionV(seed) - queryPrefix.length());
// 		}
// 		else
// 		{
// 			XavierOneDirection (result1);

// 			setBeginPositionH (seed, getEndPositionH(seed) - getEndPositionH(result1.seed));
// 			setBeginPositionV (seed, getEndPositionV(seed) - getEndPositionV(result1.seed));
// 		}

// 		std::string targetSuffix = target.substr (getEndPositionH(seed), target.length()); 	// from end seed until the end (seed included)
// 		std::string querySuffix  = query.substr  (getEndPositionV(seed), query.length());	// from end seed until the end (seed included)

// 		XavierState result2(_seed2, targetSuffix, querySuffix, scoringScheme, scoreDropOff);

// 		if (targetSuffix.length() < VECTORWIDTH || querySuffix.length() < VECTORWIDTH)
// 		{
// 			setBeginPositionH (seed, getEndPositionH(seed) + targetSuffix.length());
// 			setBeginPositionV (seed, getEndPositionV(seed) + querySuffix.length());
// 		}
// 		else
// 		{
// 			XavierOneDirection (result2);

// 			setEndPositionH (seed, getEndPositionH(seed) + getEndPositionH(result2.seed));
// 			setEndPositionV (seed, getEndPositionV(seed) + getEndPositionV(result2.seed));
// 		}

// 		// seed already updated and saved in result1
// 		// this operation sums up best and exit scores for result1 and result2 and stores them in result1
// 		result1 += result2;
// 		return std::make_pair(result1.get_best_score(), result1.get_curr_score());
// 	}
// }
