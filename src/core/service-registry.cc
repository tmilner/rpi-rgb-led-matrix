#include "core/service-registry.h"

ServiceRegistry::ServiceRegistry(const std::string &spotify_refresh_token,
                                 const std::string &spotify_client_id,
                                 const std::string &spotify_client_secret)
    : spotify_client{std::make_unique<SpotifyClient>(spotify_refresh_token,
                                                     spotify_client_id,
                                                     spotify_client_secret)},
      radio6_client{std::make_unique<Radio6Client>()},
      tfl_client{std::make_unique<TflClient>()} {}

SpotifyClient &ServiceRegistry::spotify() { return *spotify_client; }
Radio6Client &ServiceRegistry::radio6() { return *radio6_client; }
TflClient &ServiceRegistry::tfl() { return *tfl_client; }
