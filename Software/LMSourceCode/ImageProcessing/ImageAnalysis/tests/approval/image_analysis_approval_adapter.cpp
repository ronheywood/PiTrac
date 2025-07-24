/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2022-2025, Verdant Consultants, LLC.
 */

#include "image_analysis_approval_adapter.hpp"
#include "approval_test_config.hpp"
#include <filesystem>
#include <fstream>

namespace golf_sim::image_analysis::testing {

ImageAnalysisApprovalAdapter::ImageAnalysisApprovalAdapter(
    std::unique_ptr<IResultFormatter> formatter,
    std::unique_ptr<IVisualizationService> visualizer,
    std::unique_ptr<IComparisonService> comparator,
    std::unique_ptr<IDiffLauncher> diff_launcher
) : orchestrator_("ImageAnalysis"),
    formatter_(std::move(formatter)),
    visualizer_(std::move(visualizer)),
    comparator_(std::move(comparator)),
    diff_launcher_(std::move(diff_launcher)) {
}

golf_sim::shared::testing::ApprovalTestResult ImageAnalysisApprovalAdapter::RunGolfBallAnalysisTest(
    const std::string& image_filename,
    const std::string& test_name,
    infrastructure::OpenCVImageAnalyzer& analyzer,
    const std::chrono::microseconds& timestamp) {
    
    try {
        // Load the test image
        cv::Mat image = LoadPiTracImage(image_filename);
        if (image.empty()) {
            golf_sim::shared::testing::ApprovalTestResult result(test_name, "ImageAnalysis");
            result.failure_message = "Failed to load image: " + image_filename;
            return result;
        }
        
        // Perform analysis
        domain::ImageBuffer buffer{image.data, static_cast<size_t>(image.total() * image.elemSize())};
        auto analysis_result = analyzer.AnalyzeTeedBall(buffer, timestamp);
        
        // Create data generator for text approval
        auto data_generator = [this, &analysis_result](const std::string&) {
            return FormatAnalysisResult(analysis_result);
        };
        
        // Create image generator for visualization approval
        auto image_generator = [this, &image, &analysis_result](const std::string&) {
            return CreateVisualization(image, analysis_result);
        };
        
        // Run combined approval test using shared framework
        return orchestrator_.RunCombinedApprovalTest(test_name, data_generator, image_generator);
        
    } catch (const std::exception& e) {
        golf_sim::shared::testing::ApprovalTestResult result(test_name, "ImageAnalysis");
        result.failure_message = "Exception during golf ball analysis test: " + std::string(e.what());
        return result;
    }
}

golf_sim::shared::testing::ApprovalTestResult ImageAnalysisApprovalAdapter::RunMovementAnalysisTest(
    const std::vector<domain::ImageBuffer>& image_sequence,
    const domain::BallPosition& reference_position,
    const std::string& test_name,
    infrastructure::OpenCVImageAnalyzer& analyzer) {
    
    try {
        // Perform movement analysis
        auto movement_result = analyzer.AnalyzeMovement(image_sequence, reference_position);
        
        // Create data generator for movement result
        auto data_generator = [this, &movement_result](const std::string&) {
            return formatter_->FormatMovementResult(movement_result);
        };
        
        // For movement analysis, we'll just do text approval for now
        // Could be extended to include trajectory visualization
        return orchestrator_.RunTextApprovalTest(test_name, data_generator);
        
    } catch (const std::exception& e) {
        golf_sim::shared::testing::ApprovalTestResult result(test_name, "ImageAnalysis");
        result.failure_message = "Exception during movement analysis test: " + std::string(e.what());
        return result;
    }
}

cv::Mat ImageAnalysisApprovalAdapter::LoadPiTracImage(const std::string& filename) const {
    const auto& config = golf_sim::shared::testing::ApprovalTestConfig::Instance();
    std::string full_path = config.GetPiTracImagesDir() + filename;
    
    cv::Mat image = cv::imread(full_path);
    if (image.empty()) {
        throw std::runtime_error("Cannot load image: " + full_path);
    }
    
    return image;
}

std::string ImageAnalysisApprovalAdapter::FormatAnalysisResult(const domain::TeedBallResult& result) const {
    return formatter_->FormatTeedBallResult(result);
}

cv::Mat ImageAnalysisApprovalAdapter::CreateVisualization(const cv::Mat& original_image, const domain::TeedBallResult& result) const {
    cv::Mat visualization = original_image.clone();
    
    // Create a temporary file path for visualization
    std::string temp_path = orchestrator_.GetArtifactsDirectory() + "temp_visualization.png";
    
    // Use the visualization service to create the annotated image
    if (visualizer_->CreateVisualization(original_image, result, temp_path)) {
        visualization = cv::imread(temp_path);
        // Clean up temp file
        std::filesystem::remove(temp_path);
    }
    
    return visualization;
}

// Factory implementations
std::unique_ptr<ImageAnalysisApprovalAdapter> ImageAnalysisApprovalFactory::CreateStandard() {
    // Use the existing ImageAnalysis services with shared config
    auto formatter = std::make_unique<StandardApprovalFormatter>();
    auto visualizer = std::make_unique<AnnotationVisualizationService>();
    auto comparator = std::make_unique<ExactComparisonService>();
    auto diff_launcher = std::make_unique<SystemDiffLauncher>();
    
    return std::make_unique<ImageAnalysisApprovalAdapter>(
        std::move(formatter),
        std::move(visualizer),
        std::move(comparator),
        std::move(diff_launcher)
    );
}

std::unique_ptr<ImageAnalysisApprovalAdapter> ImageAnalysisApprovalFactory::CreateWithFuzzyComparison(double tolerance) {
    auto formatter = std::make_unique<StandardApprovalFormatter>();
    auto visualizer = std::make_unique<AnnotationVisualizationService>();
    auto comparator = std::make_unique<FuzzyComparisonService>(tolerance);
    auto diff_launcher = std::make_unique<SystemDiffLauncher>();
    
    return std::make_unique<ImageAnalysisApprovalAdapter>(
        std::move(formatter),
        std::move(visualizer),
        std::move(comparator),
        std::move(diff_launcher)
    );
}

std::unique_ptr<ImageAnalysisApprovalAdapter> ImageAnalysisApprovalFactory::CreateCompact() {
    auto formatter = std::make_unique<CompactApprovalFormatter>();
    auto visualizer = std::make_unique<AnnotationVisualizationService>();
    auto comparator = std::make_unique<ExactComparisonService>();
    auto diff_launcher = std::make_unique<SystemDiffLauncher>();
    
    return std::make_unique<ImageAnalysisApprovalAdapter>(
        std::move(formatter),
        std::move(visualizer),
        std::move(comparator),
        std::move(diff_launcher)
    );
}

} // namespace golf_sim::image_analysis::testing
