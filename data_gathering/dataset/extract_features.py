import os
import librosa
import numpy as np
import pandas as pd
from multiprocessing import Pool, cpu_count

FMA_DIR = "fma_medium"
GTZAN_CSV = "features_30_sec.csv"
OUTPUT_CSV = "fma_and_gtzan_merged.csv"

def extract_features(filepath):
    try:
        y, sr = librosa.load(filepath, sr=None, duration=30)
    except Exception as e:
        print(f"skipping, could not load {filepath}: {e}")
        return None

    try:
        chroma_stft = librosa.feature.chroma_stft(y=y, sr=sr)
        rms = librosa.feature.rms(y=y)
        spectral_centroid = librosa.feature.spectral_centroid(y=y, sr=sr)
        spectral_bandwidth = librosa.feature.spectral_bandwidth(y=y, sr=sr)
        rolloff = librosa.feature.spectral_rolloff(y=y, sr=sr, roll_percent=0.85)
        zero_crossing_rate = librosa.feature.zero_crossing_rate(y=y)
        harmonic, _ = librosa.effects.hpss(y)
        harmony = librosa.feature.rms(y=harmonic)
        tonnetz = librosa.feature.tonnetz(y=y, sr=sr)
        onset_env = librosa.onset.onset_strength(y=y, sr=sr)
        tempo, _ = librosa.beat.beat_track(onset_envelope=onset_env, sr=sr)
        mfcc = librosa.feature.mfcc(y=y, sr=sr, n_mfcc=20)

        features = {
            "filename": os.path.basename(filepath),
            "chroma_stft_mean": float(np.mean(chroma_stft)),
            "chroma_stft_var": float(np.var(chroma_stft)),
            "rms_mean": float(np.mean(rms)),
            "rms_var": float(np.var(rms)),
            "spectral_centroid_mean": float(np.mean(spectral_centroid)),
            "spectral_centroid_var": float(np.var(spectral_centroid)),
            "spectral_bandwidth_mean": float(np.mean(spectral_bandwidth)),
            "spectral_bandwidth_var": float(np.var(spectral_bandwidth)),
            "rolloff_mean": float(np.mean(rolloff)),
            "rolloff_var": float(np.var(rolloff)),
            "zero_crossing_rate_mean": float(np.mean(zero_crossing_rate)),
            "zero_crossing_rate_var": float(np.var(zero_crossing_rate)),
            "harmony_mean": float(np.mean(harmony)),
            "harmony_var": float(np.var(harmony)),
            "perceptr_mean": float(np.mean(tonnetz)),
            "perceptr_var": float(np.var(tonnetz)),
            "tempo": float(tempo[0]),
            **{f"mfcc{i+1}_mean": float(np.mean(mfcc[i])) for i in range(20)},
            **{f"mfcc{i+1}_var": float(np.var(mfcc[i])) for i in range(20)},
        }

        return features

    except Exception as e:
        print(f"[SKIP-FEATURES] Failed to extract features for {filepath}: {e}")
        return None

def get_all_audio_files(folder):
    file_array = []
    for root, _, filenames in os.walk(folder):
        for f in filenames:
            if f.lower().endswith(".mp3"):
                file_array.append(os.path.join(root, f))
    return file_array

def process_all_files(audio_files, num_workers=None):
    if num_workers is None:
        num_workers = max(1, cpu_count() - 1)

    print(f"number of workers: {num_workers}, number of cpu cores: {cpu_count()}")

    results = []
    processed = 0

    with Pool(num_workers) as pool:
        for features in pool.imap_unordered(extract_features, audio_files):
            processed += 1
            print(f"Processed {processed}/{len(audio_files)} files", end="\r")
            if features is not None:
                results.append(features)

    return results

def main():
    gtzan_data = pd.read_csv(GTZAN_CSV)

    audio_files = get_all_audio_files(FMA_DIR)

    fma_features = process_all_files(audio_files, num_workers=None)

    fma_df = pd.DataFrame(fma_features)

    combined_df = pd.concat([gtzan_data, fma_df], ignore_index=True)

    combined_df.to_csv(OUTPUT_CSV, index=False)


if __name__ == "__main__":
    main()
