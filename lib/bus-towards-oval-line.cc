#include "bus-towards-oval-line.h"
#include "img_utils.h"
#include <climits>
#include <iostream>
#include <mutex>
using namespace std::literals; // enables literal suffixes, e.g. 24h, 1ms, 1s.

BusTowardsOvalLine::BusTowardsOvalLine(
    std::shared_ptr<std::map<std::string, Magick::Image>> image_map,
    TflClient tflClient, ScrollingLineSettings settings)
    : ScrollingLine(settings), tflClient(tflClient),
      name{std::string("Buses Towards Oval")} {
  this->current_line = "Loading";
  this->image_map = image_map;
  this->image_key = "bus2";
  this->is_visible = true;
  auto now = std::chrono::system_clock::now();
  this->last_update = now - 5min;
}

Magick::Image *BusTowardsOvalLine::getIcon() {
  std::lock_guard<std::recursive_mutex> lock(line_mutex);
  return &(*this->image_map)[this->image_key];
}

std::string *BusTowardsOvalLine::getName() { return &this->name; }

void BusTowardsOvalLine::render(FrameCanvas *offscreen_canvas, char opacity) {
  if (!is_visible) {
    return;
  }
  if (opacity >= (CHAR_MAX / 2)) {
    this->renderLine(offscreen_canvas);
  }
  offscreen_canvas->SetPixels(0, this->y, 13, 16, 0, 0, 0);
  CopyImageToCanvas(this->getIcon(), offscreen_canvas, 0, this->y + 1, opacity);
  rgb_matrix::DrawLine(offscreen_canvas, 13, this->y, 13, this->y + 16,
                       Color(130, 100, 73));
}

void BusTowardsOvalLine::update() {
  const auto now = std::chrono::system_clock::now();

  if (((now - this->last_update) / 1s) < this->update_after_seconds) {
    return;
  }

  try {
    std::vector<TflClient::Arrival> arrivalsTowardsOval =
        this->tflClient.getBusArrivals("490014229J");
    std::vector<TflClient::Arrival> arrivalsCamberwellGreenTowardsECStopF =
        this->tflClient.getBusArrivals("490015298F");
    std::vector<TflClient::Arrival> arrivalsCamberwellGreenTowardsECStopE =
        this->tflClient.getBusArrivals("490015298E");

    std::vector<TflClient::Arrival> allArrivals;

    allArrivals.reserve(
        arrivalsTowardsOval.size() +
        arrivalsCamberwellGreenTowardsECStopF.size() +
        arrivalsCamberwellGreenTowardsECStopE.size()); // preallocate memory

    allArrivals.insert(allArrivals.end(), arrivalsTowardsOval.begin(),
                       arrivalsTowardsOval.end());
    allArrivals.insert(allArrivals.end(),
                       arrivalsCamberwellGreenTowardsECStopF.begin(),
                       arrivalsCamberwellGreenTowardsECStopF.end());
    allArrivals.insert(allArrivals.end(),
                       arrivalsCamberwellGreenTowardsECStopE.begin(),
                       arrivalsCamberwellGreenTowardsECStopE.end());

    std::sort(allArrivals.begin(), allArrivals.end(),
              [](TflClient::Arrival bus1, TflClient::Arrival bus2) {
                return bus1.secondsUntilArrival < bus2.secondsUntilArrival;
              });

    std::string busTimes = "";

    int busesAdded = 0;

    for (auto &arrival : allArrivals) {
      if (arrival.secondsUntilArrival < (30 * 60) &&
          arrival.secondsUntilArrival > 180 &&
          (arrival.busName == "185" || arrival.busName == "36" ||
           arrival.busName == "35")) {
        busesAdded++;
        busTimes.append(arrival.getDisplayString()).append(", ");
      }
      if (busesAdded >= 5) {
        break;
      }
    }

    std::cout << "\t Next busses: " << busTimes << std::endl;
    std::cout << std::endl;

    {
      std::lock_guard<std::recursive_mutex> lock(line_mutex);
      this->current_line.clear();
      this->current_line.append(busTimes);
    }
  } catch (std::runtime_error &e) {
    printf("Failed to fetch Bus Data\n");
  }
}
