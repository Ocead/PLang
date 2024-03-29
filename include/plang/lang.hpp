//
// Created by Johannes on 21.08.2022.
//

/** \page lang The PLang Language
 *
 * PLang is a markup language and file format for storing and normalizing the content of narrative works in a human and
 * machine readable way with a focus on natural syntax.
 *
 * - \subpage lang_base_path "Paths"
 *
 * PLang has four levels of abstraction,
 * which can be incrementally used to connect the information with the finished work.
 * The levels are:
 *
 * - \subpage lang_plot "Plot" <br/>
 *   Stores the actual qualitative information and links it causally
 * - \subpage lang_story "Story" <br/>
 *   Orders the plot chronologically
 * - \subpage lang_outline "Outline" <br/>
 *   Maps the parts of the story to an outline
 * - \subpage lang_text "Text" <br/>
 *   Maps the outline to the actual work
 */

#ifndef PLANG_LANG_HPP
#define PLANG_LANG_HPP

namespace plang {

    /// \brief Contains structures for parsing the Plang language
    namespace lang {

        /// \brief Valid operators in PLang
        enum class op {
            PATH,            ///< \brief <code>.</code>
            RECUR,           ///< \brief <code>...</code>
            LIST,            ///< \brief <code>,</code>
            OBJ,             ///< \brief <code>:</code>
            DECL,            ///< \brief <code>;</code>
            OBJ_NAME,        ///< \brief <code>?</code>
            SINGLE,          ///< \brief <code>!</code>
            NEGATE,          ///< \brief <code>~</code>
            STR_DELIM_SINGLE,///< \brief <code>"</code>
            STR_DELIM_DOUBLE,///< \brief <code>'</code>
            STR_L,           ///< \brief <code>L</code>
            STR_G,           ///< \brief <code>G</code>
            STR_R,           ///< \brief <code>R</code>
            STR_M,           ///< \brief <code>M</code>
            HINT_L,          ///< \brief <code>(</code>
            HINT_R,          ///< \brief <code>)</code>
            SYM_L,           ///< \brief <code>[</code>
            SYM_R,           ///< \brief <code>]</code>
            SYM,             ///< \brief <code>[]</code>
            PNT_L,           ///< \brief <code>{</code>
            PNT_R,           ///< \brief <code>}</code>
            PNT,             ///< \brief <code>{}</code>
            ID,              ///< \brief <code>*</code>
            REQ_ALL,         ///< \brief <code>*</code>
            REQ_ONE,         ///< \brief <code>+</code>
            REQ_UNQ,         ///< \brief <code>~</code>
            CAU_L,           ///< \brief <code>&lt;</code>
            CAU_R,           ///< \brief <code>&gt;</code>
            INDIR,           ///< \brief <code>&gt;&gt;</code>
            REQ_PRV,         ///< \brief <code>&lt;-</code>
            REQ_NOW,         ///< \brief <code>-&lt;</code>
            IPL_DEF,         ///< \brief <code>-&gt;</code>
            IPL_NOW,         ///< \brief <code>&gt;-</code>
            ASSIGN,          ///< \brief <code>=</code>
            COMMENT,         ///< \brief <code>//</code>
            COMMENT_L,       ///< \brief <code>/*</code>
            COMMENT_R,       ///< \brief <code>*/</code>
            ESCAPE,          ///< \brief <code>\\</code>
            CONTEXT          ///< \brief <code>\@</code>
        };

    }// namespace lang

}// namespace plang

#endif//PLANG_LANG_HPP
