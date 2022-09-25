//
// Created by Johannes on 16.08.2022.
//

/**
 * \page lang_base_path Paths
 *
 * PLang utilizes a path tree to organize a corpus' contents at the lowest level.
 * Subsequent types of entries all have a path they are located under and paths may be part of their declaration.
 * Nodes within a path are delimited with a <code>.</code> operator.
 * A leading <code>.</code> denotes a fully qualified (or absolute) path.
 * A path that omits the leading <code>.</code> and zero or more parent nodes is unqualified.
 * Unqualified paths may be used for valid declarations, as long as they are unique.
 * The context the declaration is made in, as well as the kind of entry that is declared may broaden what is considered
 * a unique path within the context of the declaration.
 * Fully qualified paths are unique be default.
 *
 * A corpus always contains at least its root path, which can be referenced by `.`. This is the only path in the corpus
 * that may not be deleted.
 *
 * ## Declaration
 *
 * The declaration of a path in PLang is the most simple one.
 * By stating a path like this
 * <table width="100%">
 * <tr>
 * <td>
 * \code .path.decl; \endcode
 * </td>
 * <td>
 * or this
 * </td>
 * <td>
 * \code path.decl; \endcode
 * </td>
 * </tr>
 * </table>
 * the path is created.
 * Fully qualified paths are always created under the root path.
 * Not fully qualified paths are created under the current context's path.
 * Redeclarations of existing paths do not change the defining path.
 *
 * Paths may be decorated with an ordinal and a description. A decorated path declaration looks like this
 * \code .path.decl (5, "Description"); \endcode
 * The ordinal controls where the path is placed in listings. The description is at the moment a hint for the user.
 * The arguments of the decoration may be omitted.
 *
 * ## Reference
 */

#ifndef PLANG_PATH_HPP
#define PLANG_PATH_HPP

#include <string>
#include <vector>
#include <plang/base.hpp>
#include <plang/corpus/detail.hpp>
#include <unordered_map>

namespace plang::detail {

    /// \brief Handles database actions for paths
    /// \see plang::root::path
    class path_manager : virtual protected plang::detail::corpus {
    private:
        /// \brief Resolves a path to persisted nodes to all viable candidates
        /// \param path Nodes from textual representation
        /// \param fully <code>true</code>, if the path should be treated as fully-qualified
        /// \return Map of fitting nodes. The key is the persisted node's id,
        /// the value is the index the node's path matches \p path up to.
        std::vector<std::tuple<pkey_t, uint_t>> partially_resolve(std::vector<string_t> const &path,
                                                                  bool_t fully) const;

        /// \brief Resolves a path to persisted nodes to the most viable candidate
        /// \param candidates List of candidates as returned by \ref path_manager::partially_resolve
        /// \param scope The id of a node to prefer from.<br/>
        /// Direct children or parents will be preferred in case of ambiguity otherwise.<br/>
        /// Setting this to the id of the root node will treat \p path as fully-qualified.
        /// \return Tuple of the most fit node and the index it matches \p path up to.<br/>
        /// \throw plang::exception::ambiguous_reference_error If two or more nodes are most viable
        std::tuple<pkey_t, uint_t> _resolve(std::vector<std::tuple<pkey_t, uint_t>> const& candidates, pkey_t scope = -1) const;

        std::vector<pkey_t> get_children(pkey_t id) const;

        /// \brief Checks, whether a path node is parent of another
        /// \param parent_id Supposed parent node id
        /// \param child_id Supposed child node id
        /// \return <code>true</code>, if the path node with id \p parent_id
        /// is parent of the parent node with id \p child_id
        bool_t is_parent_of(pkey_t parent_id, pkey_t child_id) const;

        /// Returns the passed node id, if it's persisted and the root node id otherwise
        /// \param id Node id
        /// \return The passed node id, if it's persisted and the root node id otherwise
        pkey_t or_root_id(pkey_t id) const;

        std::vector<string_t> get_full_path(pkey_t id) const;

        std::vector<string_t> get_unique_path(pkey_t id) const;

        ostream_t &print_helper(ostream_t &os, pkey_t id, format format) const;

    public:
        /// \brief Fetches a persisted path node
        /// \param id Id of the node
        /// \param texts `true`, if texts should also be returned
        /// \return The persisted node
        std::optional<path> fetch(pkey_t id, bool_t texts = false, corpus::tag<class path> = {}) const;

        /// \brief Fetches multiple persisted path nodes
        /// \param ids Ids of the node
        /// \param texts `true`, if texts should also be returned
        /// \return The persisted nodes
        std::vector<path> fetch_n(std::vector<pkey_t> const &ids,
                                  bool_t texts            = false,
                                  corpus::tag<class path> = {}) const;

        /// \brief Fetches all persisted path nodes
        /// \param texts `true`, if texts should also be returned
        /// \param limit Limits the output to up to <code>\p limit</code> entries
        /// \param offset Skips the first <code>\p offset</code> entries
        /// \return The list of persisted path nodes
        std::vector<path>
        fetch_all(bool_t texts = true, int_t limit = -1, int_t offset = 0, corpus::tag<class path> = {}) const;

        resolve_result<path> resolve(std::vector<string_t> const &path,
                                     class path const &ctx,
                                     bool_t insert           = false,
                                     bool_t texts            = false,
                                     corpus::tag<class path> = {});

        resolve_result<path> resolve(std::vector<string_t> const &path,
                                     class path const &ctx,
                                     bool_t texts            = false,
                                     corpus::tag<class path> = {}) const;

        resolve_ref_result<path> resolve(std::vector<string_t> const &path,
                                         class path &ent,
                                         class path const &ctx,
                                         bool_t insert = false,
                                         bool_t texts  = false);

        resolve_ref_result<path>
        resolve(std::vector<string_t> const &path, class path &ent, class path const &ctx, bool_t texts = false) const;

        /// \brief Inserts a new path node
        /// \param path Node to insert
        /// \param replace `true`, if existent entries should be overridden
        /// \return The inserted node
        action insert(path &path, bool_t replace = false, corpus::tag<class path> = {});

        /// \brief Updates a path node
        /// \param path Node to update
        /// \param texts `true`, if texts should also be updated
        /// \return The updated node
        action update(path &path, bool_t texts = false, corpus::tag<class path> = {});

        /// \brief Returns a representation of a persisted node
        /// \param id Id of the node to represent
        /// \param format Format options for printing
        /// \return The representation of the node
        stream_helper print(pkey_t id, format format, corpus::tag<class path> = {}) const;

        /// \brief Removes a persisted node
        /// \param path Node to remove
        /// \param cascade <code>true</code>, if dependant objects should be removed to
        /// \return The representation of the removed node
        string_t remove(path &path, bool_t cascade, corpus::tag<class path> = {});

        /// \brief Wipes the whole path tree
        /// \warning Calling this function will delete <b>all</b> content from the corpus!
        /// \details Resets the database into a state equal to being just created
        /// through the \ref plang::corpus constructor.
        /// Thus, the corpus may be used afterwards just as before the call.
        void wipe();

        /// \brief Ensures that a corpus contains a root path
        /// \details If no root path exists, this function will create it,
        /// If a root path exists, this function is a no-op.
        void ensure_root_path();

        /// \brief Returns the id of the root path
        /// \return The id of the root path
        /// \details The root node is defined by having an empty name and being its own parent
        pkey_t get_root_path_id() const;

        ~path_manager() = default;
    };

}// namespace plang::detail

#endif//PLANG_PATH_HPP
