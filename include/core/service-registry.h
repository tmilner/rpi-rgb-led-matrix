#ifndef SERVICE_REGISTRY_H
#define SERVICE_REGISTRY_H

#include "clients/radio6-client.h"
#include "clients/spotify-client.h"
#include "clients/tfl-client.h"

#include <memory>
#include <string>

class ServiceRegistry {
public:
  ServiceRegistry(const std::string &spotify_refresh_token,
                  const std::string &spotify_client_id,
                  const std::string &spotify_client_secret);

  SpotifyClient &spotify();
  const SpotifyClient &spotify() const;
  Radio6Client &radio6();
  const Radio6Client &radio6() const;
  TflClient &tfl();
  const TflClient &tfl() const;

private:
  std::unique_ptr<SpotifyClient> spotify_client;
  std::unique_ptr<Radio6Client> radio6_client;
  std::unique_ptr<TflClient> tfl_client;
};

#endif /*SERVICE_REGISTRY_H*/
