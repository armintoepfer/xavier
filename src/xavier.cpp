/**
 * File: xavier.h
 * Author: G. Guidi, E. Younis
 * Description: Xavier Main Source.
 */

#include "xavier.h"

namespace xavier
{
	AlignmentResult semi_global_alignment
	(
		const std::string& query1,
		const std::string& query2,
	    const ScoringScheme& scoringScheme,
	    const int scoreDropOff
	)
	{
		Aligner aligner( query1, query2, scoringScheme, scoreDropOff );
		return  aligner.align();
	}

	AlignmentResult seed_and_extend
	(
	 	const std::string& query1,
	 	const std::string& query2,
	    const ScoringScheme& scoringScheme,
	    const int scoreDropOff,
	    const Seed& seed
	)
	{
		// Cut query1 and query2 into left and right sequences
		// from the end of the seed position
		std::string queryHPrefix = query1.substr( 0, seed.getEndH() );
		std::string queryVPrefix = query2.substr( 0, seed.getEndV() );
		std::reverse( queryHPrefix.begin(), queryHPrefix.end() );
		std::reverse( queryVPrefix.begin(), queryVPrefix.end() );

		std::string queryHSuffix = query1.substr( seed.getEndH(), query1.length() );
		std::string queryVSuffix = query2.substr( seed.getEndV(), query2.length() );

		// Treat the two half sequences as individual alignments
		AlignmentResult left  = semi_global_alignment( queryHPrefix, queryVPrefix, scoringScheme, scoreDropOff );
		AlignmentResult right = semi_global_alignment( queryHSuffix, queryVSuffix, scoringScheme, scoreDropOff );

		// Put together the two extensions
		AlignmentResult result;
		result.bestScore = left.bestScore + right.bestScore;
		result.exitScore = left.exitScore + right.exitScore;
		result.begH = seed.getEndH() - left.endH;
		result.begV = seed.getEndV() - left.endV;
		result.endH = seed.getEndH() + right.endH;
		result.endV = seed.getEndV() + right.endV;
		// Todo Reverse left alignH and alignV
		result.matched_pair = { left.matched_pair.alignH + right.matched_pair.alignH, left.matched_pair.alignV + right.matched_pair.alignV, left.matched_pair.matches + right.matched_pair.matches };
		std::cout << "Score	" << left.bestScore << "	" << right.bestScore	<< std::endl;
		std::cout << "Score	" << left.exitScore << "	" << right.exitScore	<< std::endl;
		std::cout << "begH	" << seed.getEndH() << "	-	" << left.endH	<< std::endl;
		std::cout << "begV	" << seed.getEndV() << "	-	" << left.endV	<< std::endl;
		std::cout << "endH	" << seed.getEndH() << "	+	" << right.endH	<< std::endl;
		std::cout << "endV	" << seed.getEndV() << "	+	" << right.endV	<< std::endl;

		return result;
	}

	AlignmentResult seed_and_extend_left
	(
	 	const std::string& query1,
	 	const std::string& query2,
	    const ScoringScheme& scoringScheme,
	    const int scoreDropOff,
	    const Seed& seed
	)
	{
		// Cut query1 and query2 into left sequences
		// from the end of the seed position
		std::string queryHPrefix = query1.substr( 0, seed.getEndH() );
		std::string queryVPrefix = query2.substr( 0, seed.getEndV() );
		std::reverse( queryHPrefix.begin(), queryHPrefix.end() );
		std::reverse( queryVPrefix.begin(), queryVPrefix.end() );

		// Treat the left half as individual alignment
		AlignmentResult left  = semi_global_alignment( queryHPrefix, queryVPrefix, scoringScheme, scoreDropOff );

		// Put together the two extensions
		AlignmentResult result;
		result.bestScore = left.bestScore;
		result.exitScore = left.exitScore;
		result.begH = seed.getEndH() - left.endH;
		result.begV = seed.getEndV() - left.endV;
		result.endH = seed.getEndH();
		result.endV = seed.getEndV();
		result.matched_pair   = left.matched_pair;

		return result;
	}

	AlignmentResult seed_and_extend_right
	(
	 	const std::string& query1,
	 	const std::string& query2,
	    const ScoringScheme& scoringScheme,
	    const int scoreDropOff,
	    const Seed& seed
	)
	{
		// Cut query1 and query2 into right sequence
		// from the end of the seed position
		std::string queryHSuffix = query1.substr( seed.getBegH(), query1.length() );
		std::string queryVSuffix = query2.substr( seed.getBegV(), query2.length() );

		// Treat the right half as individual alignment
		AlignmentResult right = semi_global_alignment( queryHSuffix, queryVSuffix, scoringScheme, scoreDropOff );

		// Put together the two extensions
		AlignmentResult result;
		result.bestScore = right.bestScore;
		result.exitScore = right.exitScore;
		result.begH = seed.getBegH();
		result.begV = seed.getBegV();
		result.endH = seed.getBegH() + right.endH;
		result.endV = seed.getBegV() + right.endV;
		result.matched_pair = right.matched_pair;

		return result;
	}

} /* namespace xavier */