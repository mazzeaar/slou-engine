/**
 * @file move_generation.h
 * @author Aaron Mazzetta (amazzetta@ethz.ch)
 * @brief   the move generator works with three core functions:
 * We first generate all pseudolegal_moves, then filter them by using generate_moves and GenerateEnemyAttacks.
 *
 * @version 0.1
 * @date 2024-04-21
 *
 * @copyright Copyright (c) 2024
 *
 */

#pragma once

#include <vector>
#include <iostream>

#include "definitions.h"

#include "leapers/leapers.h"
#include "sliders/sliders.h"
#include "board.h"
#include "move.h"

#include "zobrist.h"

static bool initialized_stuff = false;

inline void initializePrecomputedStuff()
{
    magic::initMagics();
    initLeapers();
    Zobrist::initialize();
}

/**
 * @brief               This function generates all (pseudo-) possible moves for this position,
 *                      even illegal ones. We will filter the list later.
 *
 * @tparam color        Player for whom we are generating moves
 * @param move_list     A container that can store our generated moves
 * @param board         The current board representation
 */
template <type::Color color>
inline u64 pseudolegal_moves(MoveList& move_list, const Board& board)
{
    DEBUG_START;

    if ( !initialized_stuff ) {
        initializePrecomputedStuff();
        initialized_stuff = true;
    }

    if ( board.getKing(color) == 0ULL ) {
        return 0ULL;
    }

    const u64 enemy_attacks = generate_attacks<type::switchColor(color)>(board);

    leapers::pawn<color>(move_list, board);
    leapers::knight<color>(move_list, board);
    leapers::king<color>(move_list, board, enemy_attacks);

    sliders::generateMoves<type::PieceType::bishop, color>(move_list, board);
    sliders::generateMoves<type::PieceType::rook, color>(move_list, board);
    sliders::generateMoves<type::PieceType::queen, color>(move_list, board);

    DEBUG_END;
    return move_list.size();
}

template <type::Color color>
inline u64 generate_moves(MoveList& move_list, Board& board)
{
    DEBUG_START;

    pseudolegal_moves<color>(move_list, board);

    if ( move_list.size() == 0 ) {
        return 0ULL;
    }

    for ( size_t i = 0; i < move_list.size(); ) {
        board.move(move_list[i]);

        const u64 enemy_attacks = generate_attacks<type::switchColor(color)>(board);

        if ( board.isCheck(color, enemy_attacks) ) {
            board.undo(move_list[i]);
            move_list.remove(i);
        }
        else {
            board.undo(move_list[i]);
            ++i;
        }
    }

    DEBUG_END;
    return move_list.size();
}

/**
 * @brief   Generates a bitboard containing all fields that enemies can attack
 *
 * @tparam enemyColor   color of the enemy
 * @param board         a board
 * @return u64          the ORed enemy attacks
 */
template <type::Color color>
inline u64 generate_attacks(const Board& board)
{
    DEBUG_START;
    u64 attacks = 0ULL;
    const u64 occupancy = board.getOccupancy();

    attacks |= leapers::generatePawnMask<color>(board.getPawns(color));
    attacks |= leapers::generateKnightMask(board.getKnights(color));
    attacks |= leapers::generateKingMask(board.getKing(color));

    attacks |= sliders::getBitboard<type::PieceType::bishop>(board.getBishops(color), occupancy);
    attacks |= sliders::getBitboard<type::PieceType::rook>(board.getRooks(color), occupancy);
    attacks |= sliders::getBitboard<type::PieceType::queen>(board.getQueens(color), occupancy);

    DEBUG_END;
    return attacks;
}
