#include "Library.hpp"

#include "StringThings.hpp"

Library::Library() :
    CsvDb::Table("library.csv", Song::kID)
{
  // Important so that Base class can add Records that can be later dynamic
  // cast to the derivide class Song
  setNewRecordFunc([]() -> CsvDb::Record* {
    return new Song;
  });

  std::map<std::string, std::function<std::string(const Scpi&)>> temp = {
      {"SHA", [this](const Scpi& scpi) -> std::string {
         auto params = scpi.getParams();
         if (params.size() < 1)
           return "no params";
         return getSha(params.at(0));
       }},
      {"ADD", [this](const Scpi& scpi) -> std::string {
         auto params = scpi.getParams();
         if (params.size() < 1)
           return "no params";
         return std::to_string(addSong(params.at(0)));
       }},
      {"RM", [this](const Scpi& scpi) -> std::string {
         auto params = scpi.getParams();
         if (params.size() < 1)
           return "no params";
         return std::to_string(removeRecord(params.at(0)));
       }},
      {"FIND", [this](const Scpi& scpi) -> std::string {
         auto params = scpi.getParams();
         if (params.size() < 1)
           return "no params";

         std::vector<const Song*> songs = findSong(params.at(0));

         if (songs.empty())
           return "No results";

         std::string result = StringThings::vecToStr<const Song*>(songs, ",", [](const Song* song) -> std::string {
           return song->getField(Song::kARTIST) + ":" + song->getField(Song::kTITLE) + ":" + song->getField(Song::kID);
         });

         return result;
       }},
  };

  addScpiFunctions(temp);
}

Library::~Library()
{
}

bool Library::addSong(const std::string& filename)
{
  Song* song = new Song;
  song->setField(Song::kPATH, filename);

  // grab basename
  auto cleanFilename = filename.substr(filename.rfind("/") + 1, filename.length() - filename.rfind("/") - 1);
  StringThings::replace(cleanFilename, "_", " ");             // Remove underscroes
  StringThings::replaceRegex(cleanFilename, "\\.\\S*$", "");  // remove file extensions
  StringThings::replace(cleanFilename, "Karaoke", "");
  StringThings::replace(cleanFilename, "Version", "");
  StringThings::replace(cleanFilename, "KaraFun", "");

  auto tokens = StringThings::strToVec(cleanFilename, '-');

  for (auto& token : tokens)
  {
    StringThings::replaceRegex(token, "^\\s*", "");
    StringThings::replaceRegex(token, "\\s*$", "");
  }

  if (tokens.size() < 1)
    return false;

  auto sha = getSha(filename);
  if (sha.empty())
    return false;

  song->setField(Song::kID, sha);
  song->setField(Song::kTITLE, tokens.at(0));

  if (tokens.size() < 2)
    return addRecord(song);

  song->setField(Song::kARTIST, tokens.at(0));
  song->setField(Song::kTITLE, tokens.at(1));
  return addRecord(song);
}

std::vector<const Song*> Library::findSong(const std::string& criteria) const
{
  auto records = findRecord(criteria, {Song::kPATH, Song::kTITLE, Song::kARTIST});
  std::vector<const Song*> results;
  for (const CsvDb::Record* record : records)
  {
    if (const Song* song = dynamic_cast<const Song*>(record); song)
      results.push_back(dynamic_cast<const Song*>(record));
  }
  return results;
}

Song* Library::getSong(const std::string& id)
{
  auto record = getRecord(id);
  if (Song* song = dynamic_cast<Song*>(record); song)
    return dynamic_cast<Song*>(record);
  return nullptr;
}
