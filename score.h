//===========================================================================
// Title:  Xavier: High-Performance X-Drop Adaptive Banded Pairwise Alignment
// Author: G. Guidi, E. Younis, A. Zeni
// Date:   8 March 2019
//===========================================================================

#ifndef __NVCC__

#ifndef SCORE_H
#define SCORE_H

struct ScoringSchemeX
{
		short match_score;      // match
		short mismatch_score;   // substitution
		short gap_extend_score; // gap extension (indels)
		short gap_open_score;   // gap opening (indels)

		ScoringSchemeX()
				: match_score(1), mismatch_score(-1), gap_extend_score(-1), gap_open_score(-1) {
		}

		// liner gap penalty
		ScoringSchemeX(short match, short mismatch, short gap)
				: match_score(match), mismatch_score(mismatch),
					gap_extend_score(gap), gap_open_score(gap) {
		}

		// affine gap penalty
		ScoringSchemeX(short match, short mismatch, short gap_extend, short gap_open) 
				: match_score(match), mismatch_score(mismatch),
					gap_extend_score(gap_extend), gap_open_score(gap_open) {
		}
};

// return match score
inline short
scoreMatch(ScoringSchemeX const& me) {
	return me.match_score;
}

// individually set match score
inline void
setScoreMatch(ScoringSchemeX & me, short const& value) {
	me.match_score = value;
}

// return mismatch score
inline short
scoreMismatch(ScoringSchemeX const& me) {
	return me.mismatch_score;
}

// individually set mismatch score
inline void
setScoreMismatch(ScoringSchemeX & me, short const& value) {
	me.mismatch_score = value;
}

// return gap extension score
inline short
scoreGapExtend(ScoringSchemeX const& me) {
	return me.gap_extend_score;
}

// individually set gap extension score
inline void
setScoreGapExtend(ScoringSchemeX & me, short const& value) {
	me.gap_extend_score = value;
}

// return gap opening score
inline short
scoreGapOpen(ScoringSchemeX const& me) {
	return me.gap_open_score;
}

//returns the gap_open_score NB: valid only for linear gap
inline short
scoreGap(ScoringSchemeX const & me){
	return me.gap_extend_score;
}

// individually set gap opening score
inline void
setScoreGapOpen(ScoringSchemeX & me, short const& value) {
	me.gap_open_score = value;
}

// set gap opening and gap extend scores
inline void
setScoreGap(ScoringSchemeX & me, short const& value) {
	me.gap_extend_score = value;
	me.gap_open_score = value;
}

inline short
score(ScoringSchemeX const & me, char valH, char valV) {
    if (valH == valV)
        return scoreMatch(me);
    else
        return scoreMismatch(me);
}

#endif
#endif // __NVCC__