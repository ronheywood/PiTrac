/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2022-2025, Verdant Consultants, LLC.
 */

#ifdef __unix__  // Ignore in Windows environment
#include <bcm_host.h>
#endif
#include <boost/foreach.hpp>
#include <cstdlib>
#include <memory>
#include <regex>
#include <functional>

#include "logging_tools.h"
#include "gs_camera.h"
#include "gs_ui_system.h"
#include "gs_config.h"

// Having to set the constants in this way creates more entanglement than we'd like.  TBD - Re-architect
#include "libcamera_interface.h"

namespace {
// Helper function to safely get environment variable as std::string
static std::string safe_getenv(const char* varname) {
    char* buffer = nullptr;
    size_t sz = 0;
    if (_dupenv_s(&buffer, &sz, varname) == 0 && buffer != nullptr) {
        std::string value(buffer);
        free(buffer);
        return value;
    }
    return std::string();
}

// Helper function to expand environment variables in a string (supports %VAR%, $VAR, ${VAR})
static std::string expand_env_vars(const std::string& input) {
    std::string result = input;
    // Only expand if string contains % or $
    if (result.find('%') != std::string::npos) {
        // Windows-style: %VAR%
        std::regex win_env_re("%([A-Za-z0-9_]+)%");
        std::string replaced;
        std::sregex_iterator it(result.begin(), result.end(), win_env_re);
        std::sregex_iterator end;
        size_t last_pos = 0;
        while (it != end) {
            replaced.append(result, last_pos, it->position() - last_pos);
            std::string val = safe_getenv((*it)[1].str().c_str());
            replaced.append(val);
            last_pos = it->position() + it->length();
            ++it;
        }
        replaced.append(result, last_pos, std::string::npos);
        result = replaced;
    }
    if (result.find('$') != std::string::npos) {
        // POSIX-style: $VAR or ${VAR}
        std::regex posix_env_re(R"(\$\{([A-Za-z0-9_]+)\}|\$([A-Za-z0-9_]+))");
        std::string replaced;
        std::sregex_iterator it(result.begin(), result.end(), posix_env_re);
        std::sregex_iterator end;
        size_t last_pos = 0;
        while (it != end) {
            replaced.append(result, last_pos, it->position() - last_pos);
            std::string var = (*it)[1].matched ? (*it)[1].str() : (*it)[2].str();
            std::string val = safe_getenv(var.c_str());
            replaced.append(val);
            last_pos = it->position() + it->length();
            ++it;
        }
        replaced.append(result, last_pos, std::string::npos);
        result = replaced;
    }
    return result;
}
} // end anonymous namespace

namespace golf_sim {

	boost::property_tree::ptree GolfSimConfiguration::configuration_root_;

	bool GolfSimConfiguration::Initialize(const std::string& configuration_filename) {

		try {
			boost::property_tree::read_json(configuration_filename, configuration_root_);
		}
		catch (std::exception const& e)
		{
			GS_LOG_MSG(error, "GolfSimConfiguration::Initialize failed. ERROR: *** " + std::string(e.what()) + " ***");
			return false;
		}

		// Read any values that we want to set early, here at initialization
		if (!ReadValues()) {
			return false;
		}

		return true;
	}


	bool GolfSimConfiguration::ReadShotInjectionData(std::vector<GsResults>& shots,
													 int & kInterShotInjectionPauseSeconds) {
		try {
			SetConstant("gs_config.testing.kInterShotInjectionPauseSeconds", kInterShotInjectionPauseSeconds);

			// Retrirve as many shots as are defined in the json file
			boost::property_tree::ptree shots_json = configuration_root_.get_child("gs_config.testing.test_shots_to_inject");

			int shot_number = 1;
			for (boost::property_tree::ptree::iterator iter = shots_json.begin(); iter != shots_json.end(); iter++) {
			// for (boost::property_tree::ptree& shot_section : shots_json) {
				GsResults result;
				result.shot_number_ = shot_number;
				shot_number++;

				result.speed_mph_ = iter->second.get<float>("Speed", 0);
				result.hla_deg_ = iter->second.get<float>("HLA", 0);
				result.vla_deg_ = iter->second.get<float>("VLA", 0);
				result.back_spin_rpm_ = iter->second.get<int>("BackSpin", 0);
				result.side_spin_rpm_ = iter->second.get<int>("SideSpin", 0);
				result.club_type_ = GolfSimClubs::GsClubType::kNotSelected;

				shots.push_back(result);
			}
		}
		catch (std::exception const& e)
		{
			GS_LOG_MSG(error, "GolfSimConfiguration::ReadShotInjectionData failed. ERROR: *** " + std::string(e.what()) + " ***");
			return false;
		}
		return true;
	}


	// TBD - Will need to be improved to adapt to having two cameras on the same
	// Pi.
	GolfSimConfiguration::PiModel GolfSimConfiguration::GetPiModel() {
		GolfSimConfiguration::PiModel pi_model  = kRPi5;

#ifdef __unix__  // Ignore in Windows environment
		int processor_type = bcm_host_get_processor_id();

		GS_LOG_TRACE_MSG(trace, "GolfSimConfiguration - bcm_host_get_processor_id returned:" + std::to_string(processor_type));

		switch (processor_type) {
			/** NOT IMPLEMENTED YET 
			case BCM_HOST_PROCESSOR_BCM2712:
				pi_model = kRPi5;
				break;
			**/
			case BCM_HOST_PROCESSOR_BCM2837:
				pi_model = kRPi5;
				break;

			case BCM_HOST_PROCESSOR_BCM2711:
					pi_model = kRPi4;
					break;

			default:
				pi_model = kRPi5;
				break;
		}
#endif	
		return pi_model;
	}


bool GolfSimConfiguration::ReadValues() {

	// Many constants are read in the modules that own those constants.  But some are better 
	// initialized here, early, before those modules may get to be initialized (if they even
	// have an initialization).

	SetConstant("gs_config.physical_constants.kBallRadiusMeters", GolfBall::kBallRadiusMeters);

	SetConstant("gs_config.cameras.kCamera1PositionsFromExpectedBallMeters", GolfSimCamera::kCamera1PositionsFromExpectedBallMeters);
	SetConstant("gs_config.cameras.kCamera2PositionsFromExpectedBallMeters", GolfSimCamera::kCamera2PositionsFromExpectedBallMeters);
	SetConstant("gs_config.cameras.kCamera2OffsetFromCamera1OriginMeters", GolfSimCamera::kCamera2OffsetFromCamera1OriginMeters);

#ifdef __unix__  // Ignore in Windows environment

	SetConstant("gs_config.user_interface.kWebServerResultBallExposureCandidates",GsUISystem::kWebServerResultBallExposureCandidates);
	SetConstant("gs_config.user_interface.kWebServerResultSpinBall1Image", GsUISystem::kWebServerResultSpinBall1Image);
	SetConstant("gs_config.user_interface.kWebServerResultSpinBall2Image", GsUISystem::kWebServerResultSpinBall2Image);
	SetConstant("gs_config.user_interface.kWebServerResultBallRotatedByBestAngles", GsUISystem::kWebServerResultBallRotatedByBestAngles);
	SetConstant("gs_config.user_interface.kWebServerErrorExposuresImage", GsUISystem::kWebServerErrorExposuresImage);
	SetConstant("gs_config.user_interface.kWebServerBallSearchAreaImage", GsUISystem::kWebServerBallSearchAreaImage);
	
	SetConstant("gs_config.image_capture.kMaxWatchingCropWidth", LibCameraInterface::kMaxWatchingCropWidth);
	SetConstant("gs_config.image_capture.kMaxWatchingCropHeight", LibCameraInterface::kMaxWatchingCropHeight);
	SetConstant("gs_config.cameras.kCamera1Gain", LibCameraInterface::kCamera1Gain);
	SetConstant("gs_config.cameras.kCamera1HighFPSGain", LibCameraInterface::kCamera1HighFPSGain);
	SetConstant("gs_config.cameras.kCamera1Contrast", LibCameraInterface::kCamera1Contrast);
	SetConstant("gs_config.cameras.kCamera2Gain", LibCameraInterface::kCamera2Gain);

	// Let the command-line gain parameter override the .json config file parameter 
	// TBD - May want to have separate gain options?
	if (GolfSimOptions::GetCommandLineOptions().camera_gain_ > 0.0) {
		GS_LOG_MSG(info, "Overriding camera gains with value: " + std::to_string(GolfSimOptions::GetCommandLineOptions().camera_gain_) + ".");
		LibCameraInterface::kCamera1Gain = GolfSimOptions::GetCommandLineOptions().camera_gain_;
		LibCameraInterface::kCamera2Gain = GolfSimOptions::GetCommandLineOptions().camera_gain_;
	}

	SetConstant("gs_config.cameras.kCamera2CalibrateOrLocationGain", LibCameraInterface::kCamera2CalibrateOrLocationGain);	
	SetConstant("gs_config.cameras.kCamera2ComparisonGain", LibCameraInterface::kCamera2ComparisonGain);
	SetConstant("gs_config.testing.kCamera2StrobedEnvironmentGain", LibCameraInterface::kCamera2StrobedEnvironmentGain);
	SetConstant("gs_config.cameras.kCamera2Contrast", LibCameraInterface::kCamera2Contrast);
	SetConstant("gs_config.cameras.kCamera2PuttingGain", LibCameraInterface::kCamera2PuttingGain);
	SetConstant("gs_config.cameras.kCamera2PuttingContrast", LibCameraInterface::kCamera2PuttingContrast);
	SetConstant("gs_config.cameras.kCamera1StillShutterTimeuS", LibCameraInterface::kCamera1StillShutterTimeuS);
	SetConstant("gs_config.cameras.kCamera2StillShutterTimeuS", LibCameraInterface::kCamera2StillShutterTimeuS);
	SetConstant("gs_config.cameras.kCameraMotionDetectSettings", LibCameraInterface::kCameraMotionDetectSettings);

	// The web server share directory isn't really a value we want to use from the .json configuration
	// file anymore, but for now, let's allow it as a fall-back to the command line
	if (!GolfSimOptions::GetCommandLineOptions().web_server_share_dir_.empty()) {
		GsUISystem::kWebServerShareDirectory = GolfSimOptions::GetCommandLineOptions().web_server_share_dir_;
	}
	else {
		// Attempt to get the image logging directory from the .json config file
		SetConstant("gs_config.user_interface.kWebServerShareDirectory", GsUISystem::kWebServerShareDirectory);
	}

	// If the configuration file forgot to add a "/" at the end of the logging directory, we should add it here ourselves
	if (GsUISystem::kWebServerShareDirectory.back() != '/') {
		GsUISystem::kWebServerShareDirectory += '/';
	}

#endif

    // Read any environment variables that we may need

    std::string slot1_env = safe_getenv("PITRAC_SLOT1_CAMERA_TYPE");
    if (slot1_env.empty()) {
        GS_LOG_TRACE_MSG(info, "GolfSimConfiguration - PITRAC_SLOT1_CAMERA_TYPE environment variable was not set.  Assuming default of: " + std::to_string(GolfSimCamera::kSystemSlot1CameraType));
    } else {
        GolfSimCamera::kSystemSlot1CameraType = CameraHardware::string_to_camera_model(slot1_env);
    }

    std::string slot2_env = safe_getenv("PITRAC_SLOT2_CAMERA_TYPE");
    if (slot2_env.empty()) {
        if (GolfSimOptions::GetCommandLineOptions().run_single_pi_) {
            // We somewhat arbitrarily require the slot2 camera type to be set if we're running in single-pi mode.
            GS_LOG_TRACE_MSG(error, "GolfSimConfiguration - PITRAC_SLOT2_CAMERA_TYPE environment variable must be set when running in single-pi mode, but was not.  Exiting.");
            return false;
        } else {
            GS_LOG_TRACE_MSG(info, "GolfSimConfiguration - PITRAC_SLOT2_CAMERA_TYPE environment variable was not set.  Assuming default of: " + std::to_string(GolfSimCamera::kSystemSlot2CameraType));
        }
    } else {
        GolfSimCamera::kSystemSlot2CameraType = CameraHardware::string_to_camera_model(slot2_env);
    }



	return true;
}

	bool GolfSimConfiguration::PropertyExists(const std::string& value_tag) {
		// int count = configuration_root_.count(value_tag);
		boost::optional<std::string> v = configuration_root_.get_optional<std::string>(value_tag);

		return ((bool)v);
	}


	void GolfSimConfiguration::SetConstant(const std::string& tag_name, bool& constant_value) {
		try {
			constant_value = configuration_root_.get<bool>(tag_name, false);
		}
		catch (std::exception const& e)
		{
			GS_LOG_MSG(error, "GolfSimConfiguration::SetConstant failed. ERROR: *** " + std::string(e.what()) + " ***");
			constant_value = false;
		}
	}

	void GolfSimConfiguration::SetConstant(const std::string& tag_name, int& constant_value) {
		try {
			constant_value = configuration_root_.get<int>(tag_name, 0);
		}
		catch (std::exception const& e)
		{
			GS_LOG_MSG(error, "GolfSimConfiguration::SetConstant failed. ERROR: *** " + std::string(e.what()) + " ***");
			constant_value = false;
		}
	}

	void GolfSimConfiguration::SetConstant(const std::string& tag_name, long& constant_value) {
		try {
			constant_value = configuration_root_.get<long>(tag_name, 0);
		}
		catch (std::exception const& e)
		{
			GS_LOG_MSG(error, "GolfSimConfiguration::SetConstant failed. ERROR: *** " + std::string(e.what()) + " ***");
			constant_value = false;
		}
	}

	void GolfSimConfiguration::SetConstant(const std::string& tag_name, unsigned int& constant_value) {
		try {
			constant_value = configuration_root_.get<uint>(tag_name, 0);
		}
		catch (std::exception const& e)
		{
			GS_LOG_MSG(error, "GolfSimConfiguration::SetConstant failed. ERROR: *** " + std::string(e.what()) + " ***");
			constant_value = false;
		}
	}

	 void GolfSimConfiguration::SetConstant(const std::string& tag_name, float& constant_value) {
		try {
			constant_value = configuration_root_.get<float>(tag_name, 0.0);
		}
		catch (std::exception const& e)
		{
			GS_LOG_MSG(error, "GolfSimConfiguration::SetConstant failed. ERROR: *** " + std::string(e.what()) + " ***");
			constant_value = false;
		}
	}

	 void GolfSimConfiguration::SetConstant(const std::string& tag_name, double& constant_value) {
		try {
			constant_value = configuration_root_.get<double>(tag_name, 0.0);
		}
		catch (std::exception const& e)
		{
			GS_LOG_MSG(error, "GolfSimConfiguration::SetConstant failed. ERROR: *** " + std::string(e.what()) + " ***");
			constant_value = false;
		}
	}

	 void GolfSimConfiguration::SetConstant(const std::string& tag_name, std::string& constant_value) {
    try {
        constant_value = configuration_root_.get<std::string>(tag_name, "");
        // Expand environment variables in the string
        constant_value = expand_env_vars(constant_value);
    }
    catch (std::exception const& e)
    {
        GS_LOG_MSG(error, "GolfSimConfiguration::SetConstant failed. ERROR: *** " + std::string(e.what()) + " ***");
        constant_value = "";
    }
	 }

	 void GolfSimConfiguration::SetConstant(const std::string& tag_name, cv::Vec3d& vec) {
		 try {
			 int i = 0;
			 for (boost::property_tree::ptree::value_type& element : configuration_root_.get_child(tag_name)) {
				 // vec[i] = std::stod(element.second.data());
				 vec[i] = element.second.get_value<double>();
				 i++;
			 }
		 }
		 catch (std::exception const& e)
		 {
			 GS_LOG_MSG(error, "GolfSimConfiguration::SetConstant failed. ERROR: *** " + std::string(e.what()) + " ***");
		 }
	 }

	 void GolfSimConfiguration::SetConstant(const std::string& tag_name, cv::Vec2d& vec) {
		 try {
			 int i = 0;
			 for (boost::property_tree::ptree::value_type& element : configuration_root_.get_child(tag_name)) {
				 // vec[i] = std::stod(element.second.data());
				 vec[i] = element.second.get_value<double>();
				 i++;
			 }
		 }
		 catch (std::exception const& e)
		 {
			 GS_LOG_MSG(error, "GolfSimConfiguration::SetConstant failed. ERROR: *** " + std::string(e.what()) + " ***");
		 }
	 }

	 void GolfSimConfiguration::SetConstant(const std::string& tag_name, std::vector<float>& vec) {
		 try {
			 int i = 0;
			 for (boost::property_tree::ptree::value_type& element : configuration_root_.get_child(tag_name)) {
				 vec.push_back( element.second.get_value<float>() );
				 i++;
			 }
		 }
		 catch (std::exception const& e)
		 {
			 GS_LOG_MSG(error, "GolfSimConfiguration::SetConstant failed. ERROR: *** " + std::string(e.what()) + " ***");
		 }
	 }

	 void GolfSimConfiguration::SetConstant(const std::string& tag_name, std::vector<cv::Vec3d>& matrix) {
		 try {
			 int x = 0;
			 for (boost::property_tree::ptree::value_type& row : configuration_root_.get_child(tag_name))
			 {
				 int y = 0;
				 for (boost::property_tree::ptree::value_type& cell : row.second)
				 {
					 matrix[x][y] = cell.second.get_value<double>();
					 y++;
				 }
				 x++;
			 }
		 }
		 catch (std::exception const& e)
		 {
			 GS_LOG_MSG(error, "GolfSimConfiguration::SetConstant failed. ERROR: *** " + std::string(e.what()) + " ***");
		 }
	 }

	 void GolfSimConfiguration::SetConstant(const std::string& tag_name, cv::Mat& matrix) {
		 bool is_1D = (matrix.rows == 1);

		 try {
			 if (is_1D) {
				 int i = 0;
				 for (boost::property_tree::ptree::value_type& element : configuration_root_.get_child(tag_name)) {
					 matrix.at<double>(0, i) = element.second.get_value<double>();
					 i++;
				 }
			 }
			 else {
				 int x = 0;
				 for (boost::property_tree::ptree::value_type& row : configuration_root_.get_child(tag_name))
				 {
					 int y = 0;
					 for (boost::property_tree::ptree::value_type& cell : row.second)
					 {
						 matrix.at<double>(x, y) = cell.second.get_value<double>();
						 y++;
					 }
					 x++;
				 }
			 }
		 }
		 catch (std::exception const& e)
		 {
			 GS_LOG_MSG(error, "GolfSimConfiguration::SetConstant failed. ERROR: *** " + std::string(e.what()) + " ***");
		 }
	 }

	 bool GolfSimConfiguration::RemoveTreeNode(const std::string& tag_name) {

		 try {
			 std::string end_node_name = "kCamera1FocalLength";

			 if (PropertyExists(tag_name)) {
				 configuration_root_.erase(tag_name);
				 return true;
			 }
		 }
		 catch (std::exception const& e)
		 {
			 GS_LOG_MSG(error, "GolfSimConfiguration::RemoveTreeNode failed. ERROR: *** " + std::string(e.what()) + " ***");
			 return false;
		 }

		 return false;
	 }

	 bool GolfSimConfiguration::SetTreeValue(const std::string& tag_name, const cv::Vec2d& vec) {

		 boost::property_tree::ptree values_node;

		 try {
			 // RemoveTreeNode(tag_name);

			 bool node_exists = PropertyExists(tag_name);

			 if (node_exists) {
				 int i = 0;
				 for (boost::property_tree::ptree::value_type& element : configuration_root_.get_child(tag_name)) {
					 element.second.put("", vec[i++]);
				 }
			 }
			 else {
				 boost::property_tree::ptree new_node;
				 boost::property_tree::ptree element;

				 element.put("", vec[0]);
				 new_node.push_back(std::make_pair("", element));

				 element.clear();
				 element.put("", vec[1]);
				 new_node.push_back(std::make_pair("", element));

				 configuration_root_.add_child(tag_name, new_node);
			 }
		 }
		 catch (std::exception const& e)
		 {
			 GS_LOG_MSG(error, "GolfSimConfiguration::SetTreeValue failed. ERROR: *** " + std::string(e.what()) + " ***");
			 return false;
		 }

		 // TBD - Set the values

		 return true;
	 }

	 bool GolfSimConfiguration::SetTreeValue(const std::string& tag_name, const double value) {

		 // RemoveTreeNode(tag_name);

		 try {
			 configuration_root_.put(tag_name, value);
		 }
		 catch (std::exception const& e)
		 {
			 GS_LOG_MSG(error, "GolfSimConfiguration::SetTreeValue failed. ERROR: *** " + std::string(e.what()) + " ***");
			 return false;
		 }

		 // TBD - Set the values

		 return true;
	 }

	 bool GolfSimConfiguration::WriteTreeToFile(const std::string& file_name) {

		 GS_LOG_MSG(trace, "GolfSimConfiguration::WriteTreeToFile called for file_name = " + file_name);
			 
		 std::ofstream file(file_name);
		 if (file.is_open()) {
			 boost::property_tree::write_json(file, configuration_root_);
			 file.close();
		 }
		 else {
			 GS_LOG_MSG(error, "GolfSimConfiguration::WriteTreeToFile failed. Could not open file for writing.");
			 return false;
		 }

		 return true;
	 }

} // namespace golf_sim
