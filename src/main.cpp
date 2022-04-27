#include "Authenticator.hpp"
#include "DlQueue.hpp"
#include "Library.hpp"
#include "Playlist.hpp"
#include "Scpi.hpp"
#include "StringThings.hpp"
#include "TcpServer.hpp"
#include "KaraokeApp.hpp"

int main(int argc, char** argv)
{
  srand(time(NULL));

  YoutubeDl::DlQueue dlQueue;
  Library library;
  Playlist playlist(library);
  Authenticator authenticator;

  // when a download completes add it to the library
  dlQueue.setDownloadCompleteFunc([&library](const std::string& filename) {
    library.addSong(filename);
    library.saveTable();
  });

  const std::map<std::string, std::function<std::string(const Scpi&)>> subSystemMap = {
      {"DLQUEUE", [&dlQueue](const Scpi& scpi) { return dlQueue.parseScpi(scpi); }},
      {"PLAYLIST", [&playlist](const Scpi& scpi) { return playlist.parseScpi(scpi); }},
      {"LIBRARY", [&library](const Scpi& scpi) { return library.parseScpi(scpi); }},
      {"AUTH", [&authenticator](const Scpi& scpi) { return authenticator.parseScpi(scpi); }},
  };

  TcpServer server(
      [&subSystemMap](const std::vector<char>& linkVec) -> std::vector<char> {
        std::string scpiCommand(linkVec.begin(), linkVec.end());
        StringThings::replace(scpiCommand, "\n", "");

        Scpi scpi(scpiCommand);

        std::string result;
        if (scpi.getSubsystems().size() < 1)
        {
          result = "malformed scpi " + scpi.getScpiStr() + "\n";
          return std::vector(result.begin(), result.end());
        }

        auto subSystem = scpi.getSubsystems()[0];
        if (subSystemMap.find(subSystem) == subSystemMap.end())
        {
          result = "No subSystem: \"" + subSystem + "\"\n";
          return std::vector(result.begin(), result.end());
        }

        result = subSystemMap.at(subSystem)(scpi) + "\n";

        return std::vector(result.begin(), result.end());
      },
      18081);

  dlQueue.resumeDownloads();

  library.loadTable();
  authenticator.loadTable();
  playlist.loadTable();

  server.start();

  if (argc == 2 && std::string(argv[1]) == "server")
  {
    std::cout << "Starting tcp server..." << std::endl;
    std::string temp;
    std::getline(std::cin, temp);
    return 0;
  }

  return Wt::WRun(argc, argv, [&server, &dlQueue, &library, &authenticator, &playlist](const Wt::WEnvironment& env) {
    return std::make_unique<KaraokeApp>(env, dlQueue, library, authenticator, playlist);
  });
}
