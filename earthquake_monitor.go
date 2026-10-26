package main

import (
	"fmt"
	"math"
	"math/rand"
	"sync"
	"time"
)

// SeismicSensor represents a sensor node on Earth's surface
type SeismicSensor struct {
	ID        string
	Latitude  float64
	Longitude float64
	Elevation float64 // Meters above sea level
}

// SeismicEvent represents an earthquake detection payload
type SeismicEvent struct {
	EventID        string
	SensorID       string
	PWaveArrivalTime time.Time
	SWaveArrivalTime time.Time
	PeakAmplitude   float64 // Micro-meters
}

// TriangulatedOrigin represents calculated epicenter parameters
type TriangulatedOrigin struct {
	EstimatedLat  float64
	EstimatedLon  float64
	RichterMagnitude float64
	DepthKM       float64
}

type EarthquakeMonitorSystem struct {
	sensors []SeismicSensor
	events  chan SeismicEvent
	wg      sync.WaitGroup
}

func NewEarthquakeMonitorSystem() *EarthquakeMonitorSystem {
	return &EarthquakeMonitorSystem{
		sensors: []SeismicSensor{},
		events:  make(chan SeismicEvent, 100),
	}
}

func (sys *EarthquakeMonitorSystem) RegisterSensor(sensor SeismicSensor) {
	sys.sensors = append(sys.sensors, sensor)
}

// CalculateDistance returns the Great Circle Distance using Haversine Formula (KM)
func CalculateDistance(lat1, lon1, lat2, lon2 float64) float64 {
	const earthRadiusKM = 6371.0

	dLat := (lat2 - lat1) * math.Pi / 180.0
	dLon := (lon2 - lon1) * math.Pi / 180.0

	rLat1 := lat1 * math.Pi / 180.0
	rLat2 := lat2 * math.Pi / 180.0

	a := math.Sin(dLat/2)*math.Sin(dLat/2) +
		math.Sin(dLon/2)*math.Sin(dLon/2)*math.Cos(rLat1)*math.Cos(rLat2)
	c := 2 * math.Atan2(math.Sqrt(a), math.Sqrt(1-a))

	return earthRadiusKM * c
}

// SimulateSensorNetwork spawns concurrent goroutines emitting telemetry
func (sys *EarthquakeMonitorSystem) SimulateSensorNetwork(duration time.Duration) {
	fmt.Println("[SYSTEM] Initializing Seismic Sensor Network Telemetry...")
	for _, sensor := range sys.sensors {
		sys.wg.Add(1)
		go func(s SeismicSensor) {
			defer sys.wg.Done()
			ticker := time.NewTicker(400 * time.Millisecond)
			defer ticker.Stop()

			endTime := time.Now().Add(duration)
			for time.Now().Before(endTime) {
				<-ticker.C
				// Random trigger simulating micro-tremors or events
				if rand.Float64() < 0.3 {
					now := time.Now()
					pDelay := time.Duration(rand.Intn(500)) * time.Millisecond
					sDelay := pDelay + time.Duration(1000+rand.Intn(2000))*time.Millisecond

					event := SeismicEvent{
						EventID:          fmt.Sprintf("EVT-%d", rand.Intn(10000)),
						SensorID:         s.ID,
						PWaveArrivalTime: now.Add(pDelay),
						SWaveArrivalTime: now.Add(sDelay),
						PeakAmplitude:    10.0 + rand.Float64()*500.0,
					}
					sys.events <- event
				}
			}
		}(sensor)
	}
}

// ProcessEvents processes incoming wave arrivals and computes Richter magnitude
func (sys *EarthquakeMonitorSystem) ProcessEvents() {
	for event := range sys.events {
		sTimeDiff := event.SWaveArrivalTime.Sub(event.PWaveArrivalTime).Seconds()
		// S-P time difference rule of thumb: Distance (km) ~= S-P time (s) * 8 km/s
		estimatedDistanceKM := sTimeDiff * 8.4

		// Calculate estimated Richter Magnitude: M = log10(A) + 3*log10(8*dt) - 2.92
		magnitude := math.Log10(event.PeakAmplitude) + 1.11*math.Log10(estimatedDistanceKM) - 0.0018*estimatedDistanceKM

		fmt.Printf("[ALERT] Sensor %s captured %s | S-P Lag: %.2fs | Est. Distance: %.1f km | Mag: M%.2f\n",
			event.SensorID, event.EventID, sTimeDiff, estimatedDistanceKM, magnitude)
	}
}

func main() {
	rand.Seed(time.Now().UnixNano())
	sys := NewEarthquakeMonitorSystem()

	// Deploy global Earth monitoring stations
	sys.RegisterSensor(SeismicSensor{ID: "STATION-TOKYO", Latitude: 35.6762, Longitude: 139.6503, Elevation: 40.0})
	sys.RegisterSensor(SeismicSensor{ID: "STATION-LA", Latitude: 34.0522, Longitude: -118.2437, Elevation: 89.0})
	sys.RegisterSensor(SeismicSensor{ID: "STATION-QUITO", Latitude: -0.1807, Longitude: -78.4678, Elevation: 2850.0})
	sys.RegisterSensor(SeismicSensor{ID: "STATION-REYKJAVIK", Latitude: 64.1466, Longitude: -21.9426, Elevation: 15.0})

	fmt.Println("=========================================================")
	fmt.Println("       EARTH REAL-TIME SEISMIC MONITORING NETWORK        ")
	fmt.Println("=========================================================")
	fmt.Printf("Active Stations: %d\n", len(sys.sensors))

	// Run processor in background
	go sys.ProcessEvents()

	// Run simulation for 3 seconds
	sys.SimulateSensorNetwork(3 * time.Second)
	sys.wg.Wait()
	close(sys.events)

	fmt.Println("\n[SYSTEM] Seismic Telemetry Stream Closed gracefully.")
}