import os
import librosa 
import numpy as np
import csv

# config
FMA_DIR = "fma_medium" # dataset in same directory
OUTPUT_CSV = "fma_and_gtzan_merged.csv"

#feature extraction function
def extract_features(filepath):
    #import file, y is array of amplitudes, sr is sampling rate (keep the same)
    y, sr = librosa.load(filepath, sr=None, duration=30)

    #values: https://librosa.org/doc/latest/feature.html
    chroma_stft = librosa.feature.chroma_stft(y=y, sr=sr)
    rms = librosa.feature.rms(y=y)
    spectral_centroid = librosa.feature.spectral_centroid(y=y, sr=sr)
    spectral_bandwidth = librosa.feature.spectral_bandwidth(y=y, sr=sr)

    #processing
    features ={
        "chroma_stft_mean": float(np.mean(chroma_stft)),
        "chroma_stft_var": float(np.var(chroma_stft)),
        "rms_mean": float(np.mean(rms)),
        "rms_var": float(np.var(rms)),
        "spectral_centroid_mean": float(np.mean(spectral_centroid)),
        "spectral_centroid_var": float(np.var(spectral_centroid)),
        "spectral_bandwidth_mean": float(np.mean(spectral_bandwidth)),
        "spectral_bandwidth_var": float(np.var(spectral_bandwidth)),
    }
    return features

def get_all_audio_files(folder):
    # returns list of all .mp3 files in folder and subfolders
    file_array = []
    for root, _, filenames in os.walk(folder):
        for file in filenames:
            if file.lower().endswith(".mp3"):
                file_array.append(os.path.join(root, f))
    return file_array

def main():
    # the csv: filename and 8 features
    fieldnames = [
        "filename",
        "chroma_stft_mean", "chroma_stft_var",
        "rms_mean", "rms_var",
        "spectral_centroid_mean", "spectral_centroid_var",
        "spectral_bandwidth_mean", "spectral_bandwidth_var",
    ]

    audio_files = get_all_audio_files(FMA_DIR)

    #open csv for writing
    with open(OUTPUT_CSV, "w", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=fieldnames)
        writer.writeheader() # column titles

        for i, filepath in enumerate(audio_files):
            features = extract_features(filepath)
            row = {"filename": os.path.basename(filepath)}
            row.update(features)
            writer.writerow(row)

if __name__ == "__main__":
    main()

