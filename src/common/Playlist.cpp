#include "Playlist.hpp"

#include <iostream>

#include "StringThings.hpp"

const std::map<Playlist::State, std::string> Playlist::stateMap = {
    {Playlist::UNKNOWN, "UNKNOWN"},
    {Playlist::INIT, "INIT"},
    {Playlist::SKIP, "SKIP"},
    {Playlist::PLAYING, "PLAYING"},
    {Playlist::PAUSE, "PAUSE"},
};

Playlist::Playlist(const Library& library) :
    CsvDb::Table("playlist.csv", Ditty::kORDER),
    mLibrary(library),
    mLatestEnum(0),
    mCurrentState(UNKNOWN),
    mCurrentSongPath(""),
    mCurrentPoster("")
{
  setNewRecordFunc([]() -> CsvDb::Record* {
    return new Ditty;
  });

  addScpiFunctions({
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
      {"SKIP", [this](const Scpi&) -> std::string {
         skip();
         return std::to_string(mLatestEnum);
       }},
      {"STATE?", [this](const Scpi&) -> std::string {
         return stateMap.at(mCurrentState);
       }},
      {"SONGPATH?", [this](const Scpi&) -> std::string {
         return mCurrentSongPath;
       }},
      {"POSTER?", [this](const Scpi&) -> std::string {
         return mCurrentPoster;
       }},
      {"LATESTENUM?", [this](const Scpi&) -> std::string {
         return std::to_string(mLatestEnum);
       }},
  });

  setNewFileHeader({
      "ORDER",
      "SINGER",
      "ID",
      "TITLE",
      "ARTIST",
      "TAGS",
      "PATH",
  });
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
Playlist::State Playlist::getCurrentState() const
{
  return mCurrentState;
}
void Playlist::setCurrentState(Playlist::State val)
{
  mCurrentState = val;
}
const std::string& Playlist::getCurrentSongPath() const
{
  return mCurrentSongPath;
}
void Playlist::setCurrentSongPath(const std::string& val)
{
  mCurrentSongPath = val;
}
const std::string& Playlist::getCurrentPoster() const
{
  return mCurrentPoster;
}
void Playlist::setCurrentPoster(const std::string& val)
{
  mCurrentPoster = val;
}

size_t Playlist::loadTable()
{
  Table::loadTable();

  auto records = getTableRecords();
  if (records.empty())
  {
    mLatestEnum = 1000;
    return records.size();
  }

  auto lastIter = records.end();
  lastIter--;

  std::string lastKeyStr = lastIter->first;
  if (!StringThings::strTo(mLatestEnum, lastKeyStr))
  {
    mLatestEnum = 1000;
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

std::string Playlist::dittyPicture()
{
  std::string targetPath = "/usr/share/Wt";
  std::string targetFile = "pics/target" + std::to_string(rand()) + ".jpg";
  std::string targetFilename = targetPath + "/" + targetFile;

  auto results = songList();
  results.resize(5);

  static std::string dittyText;

  std::string text = StringThings::vecToStr(results, "\n");
  if (text.empty())
    text = "Empty Playlist...";

  if (text != dittyText)
  {
    std::string cmd = "convert -size 2560x1080 xc:black " + targetFilename;
    system(cmd.c_str());

    std::string fileText = "Karaok-HEE-hee Line up:\n\n" + text;
    cmd = "convert -pointsize 80 -fill white -draw 'text 60,100 \"" + fileText + "\"' " + targetFilename + " " + targetFilename;
    system(cmd.c_str());

    dittyText = text;
    mCurrentPoster = targetFile;
  }

  return mCurrentPoster;
}

void Playlist::updateQueue(bool removeFirst)
{
  auto nextDitty = getNextDitty();

  if (removeFirst)
  {
    removeRecord(*nextDitty);
    nextDitty = getNextDitty();
  }

  // Update Ditty picture
  dittyPicture();

  if (!nextDitty)
  {
    setCurrentSongPath("");
    return;
  }

  setCurrentSongPath(nextDitty->getField(Song::kPATH));
}

void Playlist::skip()
{
  auto records = getTableRecords();

  if (records.size() < 2)
    return;

  auto firstEle = records.begin();
  auto secondEle = firstEle++;
  auto temp = firstEle->second;
  firstEle->second = secondEle->second;
  secondEle->second = temp;
  setTableRecords(records);
  saveTable();
}
