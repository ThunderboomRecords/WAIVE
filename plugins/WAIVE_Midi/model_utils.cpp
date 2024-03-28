#include "model_utils.hpp"

std::vector<std::vector<int64_t>> GetInputShapes(const std::unique_ptr<Ort::Session>& session) 
{
    size_t node_count = session->GetInputCount();
    std::vector<std::vector<int64_t>> out(node_count);
    for (size_t i = 0; i < node_count; i++) 
        out[i] = session->GetInputTypeInfo(i).GetTensorTypeAndShapeInfo().GetShape();
    return out;
}

std::vector<std::vector<int64_t>> GetOutputShapes(const std::unique_ptr<Ort::Session>& session) 
{
    size_t node_count = session->GetOutputCount();
    std::vector<std::vector<int64_t>> out(node_count);
    for (size_t i = 0; i < node_count; i++) 
        out[i] = session->GetOutputTypeInfo(i).GetTensorTypeAndShapeInfo().GetShape();
    return out;
}

std::vector<std::string> GetInputNames(const std::unique_ptr<Ort::Session>& session) 
{
    Ort::AllocatorWithDefaultOptions allocator;
    size_t node_count = session->GetInputCount();
    std::vector<std::string> out(node_count);
    for (size_t i = 0; i < node_count; i++)
    {
        auto tmp = session->GetInputNameAllocated(i, allocator);
        out[i] = tmp.get();
    }
    return out;
}

std::vector<std::string> GetOutputNames(const std::unique_ptr<Ort::Session>& session) 
{
    Ort::AllocatorWithDefaultOptions allocator;
    size_t node_count = session->GetOutputCount();
    std::vector<std::string> out(node_count);
    for (size_t i = 0; i < node_count; i++)
    {
        auto tmp = session->GetOutputNameAllocated(i, allocator);
        out[i] = tmp.get();
    }
    return out;
}

void PrintModelDetails(
    std::string modelName, 
    std::vector<std::string> inputNames, 
    std::vector<std::string> outputNames,
    std::vector<std::vector<int64_t>> inputShape,
    std::vector<std::vector<int64_t>> outputShape
)
{
    std::cout << " - " << modelName << " details:" << std::endl;
    for(size_t i = 0; i < inputShape.size(); i++)
    {
        std::cout << "     Input " << i << " ['" << inputNames[i] << "'], shape (";
        for(size_t j = 0; j < inputShape[i].size(); j++)
        {
            std::cout << inputShape[i][j] << ", ";
        }
        std::cout << ")" << std::endl;
    }
    for(size_t i = 0; i < outputShape.size(); i++)
    {
        std::cout << "     Output " << i << " ['" << outputNames[i] << "'], shape (";
        for(size_t j = 0; j < outputShape[i].size(); j++)
        {
            std::cout << outputShape[i][j] << ", ";
        }
        std::cout << ")" << std::endl;
    }
}