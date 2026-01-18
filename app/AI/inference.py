#!/usr/bin/env python3
"""
Audio Genre Classification POC
Processes an audio file and classifies its genre using a trained CNN model.
"""

import sys
import os
import numpy as np
import librosa
import pickle
import tensorflow as tf
from pathlib import Path

# Set up legacy deserialization support before importing keras
os.environ['TF_CPP_MIN_LOG_LEVEL'] = '2'


# Constants matching training parameters
SR = 22050
CHUNK_DURATION = 3  # seconds
N_MELS = 128
N_FFT = 2048
HOP_LENGTH = 512

# Model paths
MODEL_DIR = Path(__file__).parent / "models"
MODEL_PATH = MODEL_DIR / "genre_cnn_model2.keras"
LABEL_ENCODER_PATH = MODEL_DIR / "genre_label_encoder2.pkl"


def extract_mel_spectrogram(signal, sr, n_mels=N_MELS, n_fft=N_FFT, hop_length=HOP_LENGTH):
    """
    Extract mel spectrogram from audio signal.
    """
    mel = librosa.feature.melspectrogram(
        y=signal,
        sr=sr,
        n_mels=n_mels,
        n_fft=n_fft,
        hop_length=hop_length
    )
    return librosa.power_to_db(mel, ref=np.max)


def predict_genre(audio_file_path):
    """
    Main function to predict genre from audio file.
    """
    # Load model and label encoder
    print(f"📦 Loading model from {MODEL_PATH}...")
    try:
        model = tf.keras.models.load_model(MODEL_PATH)
    except (ValueError, TypeError) as e:
        if 'DTypePolicy' in str(e) or 'batch_shape' in str(e):
            print("⚠️  Model format incompatibility. Rebuilding model architecture...")
            model = _build_model_from_weights(MODEL_PATH)
        else:
            raise
    
    print("✅ Model loaded")
    
    print(f"📦 Loading label encoder from {LABEL_ENCODER_PATH}...")
    with open(LABEL_ENCODER_PATH, 'rb') as f:
        label_encoder = pickle.load(f)
    print("✅ Label encoder loaded")
    
    print(f"Model output classes: {model.output_shape[-1]}")
    print(f"Label encoder classes: {label_encoder.classes_}")
    
    # Load audio (first CHUNK_DURATION seconds)
    print(f"🎵 Loading audio file: {audio_file_path}")
    signal, _ = librosa.load(
        audio_file_path,
        sr=SR,
        mono=True,
        duration=CHUNK_DURATION
    )
    print(f"   Audio loaded: {len(signal) / SR:.2f} seconds")
    
    # Extract mel spectrogram
    print(f"🎨 Extracting mel spectrogram...")
    mel_spec = extract_mel_spectrogram(signal, SR)
    
    # Match training shape: (1, H, W, 1)
    mel_spec = mel_spec[np.newaxis, ..., np.newaxis]
    print(f"   Input shape: {mel_spec.shape}")
    
    # Predict
    print(f"🔮 Running prediction...")
    pred = model.predict(mel_spec, verbose=0)
    
    # Get predicted class
    pred_class = np.argmax(pred, axis=1)[0]
    genre = label_encoder.inverse_transform([pred_class])[0]
    confidence = float(pred[0][pred_class])
    
    return genre, confidence, pred[0], label_encoder


def _build_model_from_weights(model_path):
    """
    Build a fresh model architecture and load weights from the saved file.
    This bypasses config deserialization issues.
    """
    import h5py
    
    # Load weights to infer output shape
    with h5py.File(model_path, 'r') as f:
        # Try to find the output layer to determine number of classes
        num_classes = None
        for key in f['model_weights'].keys():
            if 'dense' in key.lower():
                weights = f['model_weights'][key]
                # Get the last weight set (bias)
                bias_keys = list(weights.keys())
                if bias_keys:
                    # Last dimension of dense layer is the output size
                    last_key = bias_keys[-1]
                    num_classes = weights[last_key].shape[0]
                    break
        
        if num_classes is None:
            num_classes = 10  # Default fallback
    
    print(f"   Detected {num_classes} output classes")
    
    # Build a fresh model with the same architecture
    from tensorflow.keras import layers, models
    
    model = models.Sequential([
        layers.Input(shape=(N_MELS, 130, 1)),
        layers.Conv2D(32, (3, 3), activation='relu'),
        layers.MaxPooling2D((2, 2)),
        layers.Conv2D(64, (3, 3), activation='relu'),
        layers.MaxPooling2D((2, 2)),
        layers.Conv2D(128, (3, 3), activation='relu'),
        layers.MaxPooling2D((2, 2)),
        layers.Flatten(),
        layers.Dense(256, activation='relu'),
        layers.Dropout(0.5),
        layers.Dense(num_classes, activation='softmax')
    ])
    
    # Load weights
    model.load_weights(model_path)
    
    return model


def main():
    """Main entry point."""
    if len(sys.argv) < 2:
        # Default to blues1.mp3 if no argument provided
        audio_file_path = Path(__file__).parent.parent.parent / "data" / "blues1.mp3"
    else:
        audio_file_path = sys.argv[1]
    
    if not os.path.exists(audio_file_path):
        print(f"❌ Error: Audio file not found: {audio_file_path}", file=sys.stderr)
        sys.exit(1)
    
    # Run prediction
    genre, confidence, all_predictions, label_encoder = predict_genre(audio_file_path)
    
    # Display results
    print("\n" + "="*50)
    print(f"🎵 Predicted genre: {genre} ({confidence*100:.1f}%)")
    print("="*50)
    
    # Show top 3 predictions
    top_3_indices = np.argsort(all_predictions)[-3:][::-1]
    top_3_probs = all_predictions[top_3_indices]
    top_3_genres = label_encoder.classes_[top_3_indices]
    
    print("\nTop 3 Predictions:")
    for i, (g, p) in enumerate(zip(top_3_genres, top_3_probs), 1):
        print(f"  {i}. {g} ({p*100:.1f}%)")
    print("="*50)


if __name__ == "__main__":
    main()
