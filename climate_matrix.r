# 7_climate_matrix.r
# Earth Climate Trend & Carbon Correlation Statistical Analyzer

suppressMessages(library(stats))

generate_earth_climate_dataset <- function(num_years = 100) {
  set.seed(42)
  years <- 1925:(1925 + num_years - 1)
  
  # Base CO2 concentration starting at ~305 ppm with exponential rise
  co2_base <- 305 + 0.015 * (years - 1925)^2 + rnorm(num_years, mean = 0, sd = 1.2)
  
  # Global temperature anomaly relative to 20th century average (°C)
  temp_anomaly <- -0.2 + 0.012 * (years - 1925) + 0.0001 * (years - 1925)^2 + rnorm(num_years, mean = 0, sd = 0.15)
  
  # Sea Level Rise (mm cumulative)
  sea_level <- 0 + 1.8 * (years - 1925) + rnorm(num_years, mean = 0, sd = 3.0)
  
  data.frame(
    Year = years,
    CO2_ppm = round(co2_base, 2),
    TempAnomaly_C = round(temp_anomaly, 3),
    SeaLevel_mm = round(sea_level, 2)
  )
}

analyze_climate_correlations <- function(df) {
  cat("=================================================================\n")
  cat("        EARTH HISTORICAL CLIMATE & CO2 CORRELATION MATRIX        \n")
  cat("=================================================================\n\n")
  
  cor_matrix <- cor(df[, c("CO2_ppm", "TempAnomaly_C", "SeaLevel_mm")])
  print(round(cor_matrix, 4))
  cat("\n-----------------------------------------------------------------\n")
}

fit_climate_models <- function(df) {
  cat("\n--- Linear Model: Temperature Anomaly vs CO2 Concentration ---\n")
  model_temp <- lm(TempAnomaly_C ~ CO2_ppm, data = df)
  print(summary(model_temp))
  
  cat("\n--- Polynomial Model: Sea Level Rise over Time ---\n")
  model_sea <- lm(SeaLevel_mm ~ poly(Year, 2), data = df)
  print(summary(model_sea))
}

run_climate_analysis <- function() {
  dataset <- generate_earth_climate_dataset(100)
  
  cat("First 5 entries of Earth Climate Records:\n")
  print(head(dataset, 5))
  cat("\nLast 5 entries of Earth Climate Records:\n")
  print(tail(dataset, 5))
  cat("\n")
  
  analyze_climate_correlations(dataset)
  fit_climate_models(dataset)
}

run_climate_analysis()