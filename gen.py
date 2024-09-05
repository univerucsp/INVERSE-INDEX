import os
import random

# Tamaño total en bytes (20 GB)
total_size = 20 * 1024 * 1024 * 1024 
# Número de archivos
num_files = 10
# Tamaño por archivo
size_per_file = total_size // num_files

# Cargar una lista de palabras reales desde un archivo
def load_word_list():
    with open("/usr/share/dict/words", "r") as f:  # Archivo de diccionario en Linux
        words = f.read().splitlines()
    return words

# Cargar listas de palabras prohibidas para cada archivo
def load_prohibited_words_for_files():
    return {
        f"file_{i+1}.txt": {"washerwoman", "nervier", "frowzier"} if i % 2 == 0 else set()
        for i in range(num_files)
    }

# Generar texto utilizando un subconjunto específico de palabras reales y excluyendo palabras prohibidas
def generate_real_text(size, word_list, prohibited_words):
    text = []
    current_size = 0

    while current_size < size:
        # Elegir una palabra aleatoria del subconjunto de palabras permitidas
        word = random.choice(word_list)
        while word in prohibited_words:
            word = random.choice(word_list)  
        text.append(word)
        current_size += len(word) + 1  

    return ' '.join(text)

# Directorio donde se crearán los archivos
output_dir = "generated_files"
os.makedirs(output_dir, exist_ok=True)

word_list = load_word_list()
prohibited_words_by_file = load_prohibited_words_for_files()

# Seleccionar x numero de palabras aleatorias de la lista de palabras
sample_size = 300
selected_words = random.sample(word_list, min(sample_size, len(word_list)))

for i in range(num_files):
    file_name = f"file_{i+1}.txt"
    file_path = os.path.join(output_dir, file_name)
    prohibited_words = prohibited_words_by_file[file_name]
    
    with open(file_path, 'w') as f:
        remaining_size = size_per_file
        chunk_size = 1024 * 1024  # 1 MB chunks

        while remaining_size > 0:
            write_size = min(chunk_size, remaining_size)
            real_text = generate_real_text(write_size, selected_words, prohibited_words)
            f.write(real_text)
            remaining_size -= write_size

    print(f"Archivo {file_path} creado con éxito.")

print("Se generaron los archivos con las x numero de palabras seleccionadas.")

