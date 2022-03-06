#include "Playlist.hpp"

#include <iostream>

#include "StringThings.hpp"

Playlist::Playlist(const Library& library) :
    CsvDb::Table("playlist.csv", Ditty::kORDER),
    mLibrary(library),
    mLatestEnum(0)
{
  setNewRecordFunc([]() -> CsvDb::Record* {
    return new Ditty;
  });

  std::map<std::string, std::function<std::string(const Scpi&)>> temp = {
      {"ADD", [this](const Scpi& scpi) -> std::string {
         auto params = scpi.getParams();
         if (params.size() != 2)
           return "no params";

         std::string songHash;
         std::string singer;

         scpi.getParam(songHash, 0);
         scpi.getParam(singer, 1);

         return std::to_string(addDitty(songHash, singer));
       }},
      {"NEXTSONG", [this](const Scpi&) -> std::string {
         auto ditty = getNextDitty();
         auto result = ditty->getField(Ditty::kORDER) + ":" +
                       ditty->getField(Ditty::kID) + ":" +
                       ditty->getField(Ditty::kSINGER);
         result += ":" + std::to_string(removeRecord(*ditty));
         saveTable();
         return result;
       }},
      {"LIST", [this](const Scpi&) -> std::string {
         auto results = songList();
         return StringThings::vecToStr(results, "\n");
       }},
  };

  addScpiFunctions(temp);
}

Playlist::~Playlist()
{
}

size_t Playlist::getLatestEnum() const
{
  return mLatestEnum;
}
void Playlist::setLatestEnum(size_t val)
{
  mLatestEnum = val;
}

size_t Playlist::loadTable()
{
  Table::loadTable();

  auto records = getTableRecords();
  if (records.empty())
    return records.size();

  auto lastIter = records.end();
  lastIter--;

  std::string lastKeyStr = lastIter->first;
  if (!StringThings::strTo(mLatestEnum, lastKeyStr))
  {
    mLatestEnum = 0;
    clearTable();
  }

  mLatestEnum++;

  return getTableRecords().size();
}

bool Playlist::addDitty(const std::string& id, const std::string& singer)
{
  // Ensure the id exist in the library
  auto records = mLibrary.getTableRecords();
  if (records.find(id) == records.end())
    return false;

  auto record = records.at(id);

  auto song = dynamic_cast<Song*>(record);
  if (!song)
    return false;

  Song* ditty = new Ditty;
  *ditty = *song;
  ditty->setField(Ditty::kORDER, std::to_string(mLatestEnum++));
  ditty->setField(Ditty::kSINGER, singer);

  if (!addRecord(ditty))
    return false;

  saveTable();
  return true;
}

Ditty* Playlist::getNextDitty()
{
  if (getTableRecords().empty())
    return nullptr;

  auto firstDittyIter = getTableRecords().begin();
  auto ditty = dynamic_cast<Ditty*>(firstDittyIter->second);
  if (ditty)
  {
    //    removeRecord(firstDittyIter->first);
    saveTable();
  }
  return ditty;
}

std::vector<std::string> Playlist::songList() const
{
  std::vector<std::string> result;
  for (const auto& [id, ditty] : getTableRecords())
  {
    std::string text;
    std::string title;
    std::string artist;
    std::string singer;
    if (ditty)
    {
      title = ditty->getField(Ditty::kTITLE);
      StringThings::replace(title, "'", "");

      artist = ditty->getField(Ditty::kARTIST);
      StringThings::replace(artist, "'", "");

      singer = ditty->getField(Ditty::kSINGER);
      StringThings::replace(singer, "'", "");
    }
    text = singer + " : " + artist + " - " + title;
    result.push_back(text);
  }
  return result;
}
