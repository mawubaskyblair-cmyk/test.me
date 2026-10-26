/**
 * 6_climate_data_pipeline.ts
 * Earth Global Climate & Ocean Temperature Anomaly Processing Pipeline
 */

export interface ClimateReading {
    stationId: string;
    timestamp: Date;
    latitude: number;
    longitude: number;
    surfaceTempCelsius: number;
    co2Ppm: number;
    seaLevelAnomalyMm: number;
}

export interface ProcessingMetrics {
    totalRecordsProcessed: number;
    anomalyCount: number;
    averageTempCelsius: number;
    maxCo2Level: number;
}

export class ClimateDataValidator {
    public static validateReading(reading: ClimateReading): boolean {
        if (reading.latitude < -90 || reading.latitude > 90) return false;
        if (reading.longitude < -180 || reading.longitude > 180) return false;
        // Temperature range bounds for planet Earth (-90°C to +60°C)
        if (reading.surfaceTempCelsius < -90 || reading.surfaceTempCelsius > 60) return false;
        if (reading.co2Ppm < 150 || reading.co2Ppm > 2000) return false;
        return true;
    }
}

export class ClimatePipeline {
    private readingsStore: ClimateReading[] = [];
    private baselineTemperature: number = 14.0; // Earth pre-industrial mean (°C)

    public async ingestStream(readings: ClimateReading[]): Promise<void> {
        for (const reading of readings) {
            if (ClimateDataValidator.validateReading(reading)) {
                this.readingsStore.push(reading);
            } else {
                console.warn(`[INVALID DATA DISCARDED]: Station ${reading.stationId}`);
            }
        }
    }

    public detectThermalAnomalies(thresholdDeltaCelsius: number): ClimateReading[] {
        return this.readingsStore.filter(r => 
            Math.abs(r.surfaceTempCelsius - this.baselineTemperature) >= thresholdDeltaCelsius
        );
    }

    public computeSummaryMetrics(): ProcessingMetrics {
        if (this.readingsStore.length === 0) {
            return { totalRecordsProcessed: 0, anomalyCount: 0, averageTempCelsius: 0, maxCo2Level: 0 };
        }

        let totalTemp = 0;
        let maxCo2 = 0;
        let anomalyCount = 0;

        for (const r of this.readingsStore) {
            totalTemp += r.surfaceTempCelsius;
            if (r.co2Ppm > maxCo2) maxCo2 = r.co2Ppm;
            if (Math.abs(r.surfaceTempCelsius - this.baselineTemperature) > 2.0) {
                anomalyCount++;
            }
        }

        return {
            totalRecordsProcessed: this.readingsStore.length,
            anomalyCount,
            averageTempCelsius: parseFloat((totalTemp / this.readingsStore.length).toFixed(2)),
            maxCo2Level: maxCo2
        };
    }
}

// Data generator helper
function generateDummyEarthData(count: number): ClimateReading[] {
    const readings: ClimateReading[] = [];
    const stations = ["PACIFIC-BUOY-01", "ATLANTIC-STN-09", "ARCTIC-OBS-03", "SAHARA-TOWER-02"];

    for (let i = 0; i < count; i++) {
        readings.push({
            stationId: stations[i % stations.length],
            timestamp: new Date(Date.now() - i * 3600000),
            latitude: Number((Math.random() * 180 - 90).toFixed(4)),
            longitude: Number((Math.random() * 360 - 180).toFixed(4)),
            surfaceTempCelsius: Number((10 + Math.random() * 12).toFixed(2)),
            co2Ppm: Number((410 + Math.random() * 25).toFixed(1)),
            seaLevelAnomalyMm: Number((3.2 + Math.random() * 2.1).toFixed(2))
        });
    }
    return readings;
}

async function runPipeline() {
    console.log("=========================================================");
    console.log("         EARTH CLIMATE METRICS PROCESSING PIPELINE       ");
    console.log("=========================================================");

    const pipeline = new ClimatePipeline();
    const rawData = generateDummyEarthData(50);

    console.log(`Ingesting ${rawData.length} climate data packets...`);
    await pipeline.ingestStream(rawData);

    const metrics = pipeline.computeSummaryMetrics();
    console.log("\n--- Aggregate Processing Summary ---");
    console.log(`Total Records    : ${metrics.totalRecordsProcessed}`);
    console.log(`Mean Global Temp : ${metrics.averageTempCelsius} °C`);
    console.log(`Peak CO2 Level   : ${metrics.maxCo2Level} ppm`);
    console.log(`Thermal Anomalies: ${metrics.anomalyCount}`);
}

runPipeline();