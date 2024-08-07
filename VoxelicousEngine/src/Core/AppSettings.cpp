#include "AppSettings.h"

#include "yaml-cpp/yaml.h"

#include <fstream>
#include <iostream>

namespace VoxelicousEngine
{
	static void CreateDirectoriesIfNeeded(const std::filesystem::path& path)
	{
		if (const std::filesystem::path directory = path.parent_path(); !exists(directory))
			create_directories(directory);
	}

	AppSettings::AppSettings(std::filesystem::path filepath)
		: m_FilePath(std::move(filepath))
	{
		Deserialize();
	}

	void AppSettings::Serialize()
	{
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Hazel Application Settings";
		out << YAML::Value;

		out << YAML::BeginMap;
		for (const auto& [key, value] : m_Settings)
			out << YAML::Key << key << YAML::Value << value;

		out << YAML::EndMap;

		out << YAML::EndSeq;

		CreateDirectoriesIfNeeded(m_FilePath);
		std::ofstream fout(m_FilePath);
		fout << out.c_str();

		fout.close();
	}

	bool AppSettings::Deserialize()
	{
		std::ifstream stream(m_FilePath);
		if (!stream.good())
			return false;

		std::stringstream strStream;
		strStream << stream.rdbuf();

		YAML::Node data = YAML::Load(strStream.str());

		auto settings = data["Hazel Application Settings"];
		if (!settings)
			return false;

		for (auto it = settings.begin(); it != settings.end(); it++)
		{
			const auto& key = it->first.as<std::string>();
			const auto& value = it->second.as<std::string>();
			m_Settings[key] = value;
		}

		stream.close();
		return true;
	}

	bool AppSettings::HasKey(const std::string_view key) const
	{
		return m_Settings.contains(std::string(key));
	}

	std::string AppSettings::Get(const std::string_view name, const std::string& defaultValue) const
	{
		if (!HasKey(name))
			return defaultValue;

		return m_Settings.at(std::string(name));
	}

	float AppSettings::GetFloat(const std::string_view name, const float defaultValue) const
	{
		if (!HasKey(name))
			return defaultValue;

		const std::string& string = m_Settings.at(std::string(name));
		return std::stof(string);
	}

	int AppSettings::GetInt(const std::string_view name, const int defaultValue) const
	{
		if (!HasKey(name))
			return defaultValue;

		const std::string& string = m_Settings.at(std::string(name));
		return std::stoi(string);
	}

	void AppSettings::Set(const std::string_view name, const std::string_view value)
	{
		m_Settings[std::string(name)] = value;
	}

	void AppSettings::SetFloat(const std::string_view name, const float value)
	{
		m_Settings[std::string(name)] = std::to_string(value);
	}

	void AppSettings::SetInt(const std::string_view name, const int value)
	{
		m_Settings[std::string(name)] = std::to_string(value);
	}

}