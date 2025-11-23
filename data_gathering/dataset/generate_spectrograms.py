import os
import librosa
import librosa.display
import matplotlib.pyplot as plt
import numpy as np
from multiprocessing import Pool, cpu_count

FMA_DIR = "fma_medium"
OUTPUT_DIR = "fma_spectrograms"

if not os.path.exists(OUTPUT_DIR):
    os.makedirs(OUTPUT_DIR)

def generate_spectrogram(filepath):
    try:
        y, sr = librosa.load(filepath, sr=None, duration=30)
    except Exception as e:
        print(f"skipping, could not load {filepath}: {e}")
        return None

    try:
        D = librosa.amplitude_to_db(np.abs(librosa.stft(y)), ref=np.max)
        
        output_path = os.path.join(OUTPUT_DIR, os.path.basename(filepath).replace(".mp3", ".png"))
        
        plt.figure(figsize=(10, 4))
        librosa.display.specshow(D, sr=sr, x_axis='time', y_axis='log')
        plt.colorbar(format='%+2.0f dB')
        plt.title(f'Spectrogram: {os.path.basename(filepath)}')
        plt.savefig(output_path, bbox_inches='tight', pad_inches=0)
        plt.close()
        print(f"Saved spectrogram for {os.path.basename(filepath)}")
    except Exception as e:
        print(f"could not process {filepath}: {e}")

def get_all_audio_files(folder):
    file_array = []
    for root, _, filenames in os.walk(folder):
        for f in filenames:
            if f.lower().endswith(".mp3"):
                file_array.append(os.path.join(root, f))
    return file_array

def process_audio_files_in_parallel(audio_files, num_workers=None):
    if num_workers is None:
        num_workers = max(1, cpu_count() - 1)
    
    print(f"Using {num_workers} workers to process {len(audio_files)} files.")
    
    with Pool(num_workers) as pool:
        
        pool.map(generate_spectrogram, audio_files)

def main():
    audio_files = get_all_audio_files(FMA_DIR)
    
    process_audio_files_in_parallel(audio_files, 4)

if __name__ == "__main__":
    main()
