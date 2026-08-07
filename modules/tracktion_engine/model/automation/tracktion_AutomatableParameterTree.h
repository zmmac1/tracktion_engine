/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2024
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

namespace tracktion::inline engine {

class AutomatableParameterTree
{
public:
    AutomatableParameterTree()  : rootNode (new TreeNode ("root"))
    {
    }

    enum NodeType
    {
        Parameter,
        Group
    };

    struct TreeNode
    {
        TreeNode (const yup::String& groupName)
            : name (groupName), type (AutomatableParameterTree::Group)
        {}

        TreeNode (const yup::ReferenceCountedObjectPtr<AutomatableParameter>& param)
            : parameter (param), type (AutomatableParameterTree::Parameter)
        {
            jassert (parameter.get() != nullptr);
        }

        yup::String name;
        const yup::ReferenceCountedObjectPtr<AutomatableParameter> parameter;
        NodeType type;
        yup::OwnedArray<TreeNode> subNodes;
        TreeNode* parent = nullptr;

        void addSubNode (TreeNode* node)
        {
            jassert (node != nullptr);
            subNodes.add (node);
            node->parent = this;
        }

        yup::String getGroupName() const         { return name; }

        YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TreeNode)
    };

    //==============================================================================
    TreeNode* getNodeForParameter (AutomatableParameter& param) const
    {
        return getNodeForParameter (rootNode.get(), param);
    }

    void clear()
    {
        rootNode = std::make_unique<TreeNode> ("root");
    }

    std::unique_ptr<TreeNode> rootNode;

private:
    static TreeNode* getNodeForParameter (TreeNode* node, AutomatableParameter& param)
    {
        for (auto subNode : node->subNodes)
        {
            if (subNode->type == Parameter)
            {
                if (subNode->parameter.get() == &param)
                    return subNode;
            }
            else if (subNode->type == Group)
            {
                if (auto res = getNodeForParameter (subNode, param))
                    return res;
            }
        }

        return {};
    }

    YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AutomatableParameterTree)
};

} // namespace tracktion::inline engine
