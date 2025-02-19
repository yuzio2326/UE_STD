#pragma once

#ifdef _MSC_VER
#   undef ASSIMP_API

//////////////////////////////////////////////////////////////////////////
/* Define 'ASSIMP_BUILD_DLL_EXPORT' to build a DLL of the library */
//////////////////////////////////////////////////////////////////////////
#   ifdef ASSIMP_BUILD_DLL_EXPORT
#       define ASSIMP_API __declspec(dllexport)
#       define ASSIMP_API_WINONLY __declspec(dllexport)
#       pragma warning (disable : 4251)

    //////////////////////////////////////////////////////////////////////////
    /* Define 'ASSIMP_DLL' before including Assimp to link to ASSIMP in
     * an external DLL under Windows. Default is static linkage. */
     //////////////////////////////////////////////////////////////////////////
#   elif (defined ASSIMP_DLL)
#       define ASSIMP_API __declspec(dllimport)
#       define ASSIMP_API_WINONLY __declspec(dllimport)
#   else
#       define ASSIMP_API
#       define ASSIMP_API_WINONLY
#   endif

    /* Force the compiler to inline a function, if possible
     */
#   define AI_FORCE_INLINE __forceinline

     /* Tells the compiler that a function never returns. Used in code analysis
      * to skip dead paths (e.g. after an assertion evaluated to false). */
#   define AI_WONT_RETURN __declspec(noreturn)

#elif defined(SWIG)

/* Do nothing, the relevant defines are all in AssimpSwigPort.i */

#else

#   define AI_WONT_RETURN

#   define ASSIMP_API __attribute__ ((visibility("default")))
#   define ASSIMP_API_WINONLY
#   define AI_FORCE_INLINE inline
#endif // (defined _MSC_VER)


# define C_STRUCT struct
# define aiString string

struct ASSIMP_API aiNode
{
    /** The name of the node.
     *
     * The name might be empty (length of zero) but all nodes which
     * need to be referenced by either bones or animations are named.
     * Multiple nodes may have the same name, except for nodes which are referenced
     * by bones (see #aiBone and #aiMesh::mBones). Their names *must* be unique.
     *
     * Cameras and lights reference a specific node by name - if there
     * are multiple nodes with this name, they are assigned to each of them.
     * <br>
     * There are no limitations with regard to the characters contained in
     * the name string as it is usually taken directly from the source file.
     *
     * Implementations should be able to handle tokens such as whitespace, tabs,
     * line feeds, quotation marks, ampersands etc.
     *
     * Sometimes assimp introduces new nodes not present in the source file
     * into the hierarchy (usually out of necessity because sometimes the
     * source hierarchy format is simply not compatible). Their names are
     * surrounded by @verbatim <> @endverbatim e.g.
     *  @verbatim<DummyRootNode> @endverbatim.
     */
    C_STRUCT aiString mName;

    /** The transformation relative to the node's parent. */
    C_STRUCT aiMatrix4x4 mTransformation;

    /** Parent node. NULL if this node is the root node. */
    C_STRUCT aiNode* mParent;

    /** The number of child nodes of this node. */
    unsigned int mNumChildren;

    /** The child nodes of this node. NULL if mNumChildren is 0. */
    C_STRUCT aiNode** mChildren;

    /** The number of meshes of this node. */
    unsigned int mNumMeshes;

    /** The meshes of this node. Each entry is an index into the
      * mesh list of the #aiScene.
      */
    unsigned int* mMeshes;

    /** Metadata associated with this node or NULL if there is no metadata.
      *  Whether any metadata is generated depends on the source file format. See the
      * @link importer_notes @endlink page for more information on every source file
      * format. Importers that don't document any metadata don't write any.
      */
    C_STRUCT aiMetadata* mMetaData;

#ifdef __cplusplus
    /** Constructor */
    aiNode();

    /** Construction from a specific name */
    explicit aiNode(const std::string& name);

    /** Destructor */
    ~aiNode();

    /** Searches for a node with a specific name, beginning at this
     *  nodes. Normally you will call this method on the root node
     *  of the scene.
     *
     *  @param name Name to search for
     *  @return NULL or a valid Node if the search was successful.
     */
    inline
        const aiNode* FindNode(const aiString& name) const {
        return FindNode(name.data);
    }

    inline
        aiNode* FindNode(const aiString& name) {
        return FindNode(name.data);
    }

    const aiNode* FindNode(const char* name) const;

    aiNode* FindNode(const char* name);

    /**
     * @brief   Will add new children.
     * @param   numChildren  Number of children to add.
     * @param   children     The array with pointers showing to the children.
     */
    void addChildren(unsigned int numChildren, aiNode** children);
#endif // __cplusplus
};
struct aiNodeAnim {
    /** The name of the node affected by this animation. The node
     *  must exist and it must be unique.*/
    C_STRUCT aiString mNodeName;

    /** The number of position keys */
    unsigned int mNumPositionKeys;

    /** The position keys of this animation channel. Positions are
     * specified as 3D vector. The array is mNumPositionKeys in size.
     *
     * If there are position keys, there will also be at least one
     * scaling and one rotation key.*/
    C_STRUCT aiVectorKey* mPositionKeys;

    /** The number of rotation keys */
    unsigned int mNumRotationKeys;

    /** The rotation keys of this animation channel. Rotations are
     *  given as quaternions,  which are 4D vectors. The array is
     *  mNumRotationKeys in size.
     *
     * If there are rotation keys, there will also be at least one
     * scaling and one position key. */
    C_STRUCT aiQuatKey* mRotationKeys;

    /** The number of scaling keys */
    unsigned int mNumScalingKeys;

    /** The scaling keys of this animation channel. Scalings are
     *  specified as 3D vector. The array is mNumScalingKeys in size.
     *
     * If there are scaling keys, there will also be at least one
     * position and one rotation key.*/
    C_STRUCT aiVectorKey* mScalingKeys;

    /** Defines how the animation behaves before the first
     *  key is encountered.
     *
     *  The default value is aiAnimBehaviour_DEFAULT (the original
     *  transformation matrix of the affected node is used).*/
    C_ENUM aiAnimBehaviour mPreState;

    /** Defines how the animation behaves after the last
     *  key was processed.
     *
     *  The default value is aiAnimBehaviour_DEFAULT (the original
     *  transformation matrix of the affected node is taken).*/
    C_ENUM aiAnimBehaviour mPostState;

#ifdef __cplusplus
    aiNodeAnim() AI_NO_EXCEPT
        : mNumPositionKeys(0)
        , mPositionKeys(nullptr)
        , mNumRotationKeys(0)
        , mRotationKeys(nullptr)
        , mNumScalingKeys(0)
        , mScalingKeys(nullptr)
        , mPreState(aiAnimBehaviour_DEFAULT)
        , mPostState(aiAnimBehaviour_DEFAULT) {
        // empty
    }

    ~aiNodeAnim() {
        delete[] mPositionKeys;
        delete[] mRotationKeys;
        delete[] mScalingKeys;
    }
#endif // __cplusplus
};


// ---------------------------------------------------------------------------
/** An animation consists of key-frame data for a number of nodes. For
 *  each node affected by the animation a separate series of data is given.*/
struct aiAnimation {
    /** The name of the animation. If the modeling package this data was
     *  exported from does support only a single animation channel, this
     *  name is usually empty (length is zero). */
    C_STRUCT aiString mName;

    /** Duration of the animation in ticks.  */
    double mDuration;

    /** Ticks per second. 0 if not specified in the imported file */
    double mTicksPerSecond;

    /** The number of bone animation channels. Each channel affects
     *  a single node. */
    unsigned int mNumChannels;

    /** The node animation channels. Each channel affects a single node.
     *  The array is mNumChannels in size. */
    C_STRUCT aiNodeAnim** mChannels;


    /** The number of mesh animation channels. Each channel affects
     *  a single mesh and defines vertex-based animation. */
    unsigned int mNumMeshChannels;

    /** The mesh animation channels. Each channel affects a single mesh.
     *  The array is mNumMeshChannels in size. */
    C_STRUCT aiMeshAnim** mMeshChannels;

    /** The number of mesh animation channels. Each channel affects
     *  a single mesh and defines morphing animation. */
    unsigned int mNumMorphMeshChannels;

    /** The morph mesh animation channels. Each channel affects a single mesh.
     *  The array is mNumMorphMeshChannels in size. */
    C_STRUCT aiMeshMorphAnim** mMorphMeshChannels;

#ifdef __cplusplus
    aiAnimation() AI_NO_EXCEPT
        : mDuration(-1.)
        , mTicksPerSecond(0.)
        , mNumChannels(0)
        , mChannels(nullptr)
        , mNumMeshChannels(0)
        , mMeshChannels(nullptr)
        , mNumMorphMeshChannels(0)
        , mMorphMeshChannels(nullptr) {
        // empty
    }

    ~aiAnimation() {
        // DO NOT REMOVE THIS ADDITIONAL CHECK
        if (mNumChannels && mChannels) {
            for (unsigned int a = 0; a < mNumChannels; a++) {
                delete mChannels[a];
            }

            delete[] mChannels;
        }
        if (mNumMeshChannels && mMeshChannels) {
            for (unsigned int a = 0; a < mNumMeshChannels; a++) {
                delete mMeshChannels[a];
            }

            delete[] mMeshChannels;
        }
        if (mNumMorphMeshChannels && mMorphMeshChannels) {
            for (unsigned int a = 0; a < mNumMorphMeshChannels; a++) {
                delete mMorphMeshChannels[a];
            }

            delete[] mMorphMeshChannels;
        }
    }
#endif // __cplusplus
};

