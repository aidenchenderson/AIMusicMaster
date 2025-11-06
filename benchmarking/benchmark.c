#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#define PS_BUFFER_SIZE 1024

void get_resource_usage(FILE* output_file);

/**
 * @brief program to benchmark ollama models on the RPi5 16GB
 * @param argc input argument count
 * @param argv input argument vector
 */
int main(int argc, char** argv) {
	// load the models to be tested
	FILE* models_file = fopen("models.txt", "r");
	if (models_file == NULL) {
		printf("Error opening models file\n");
		return 1;
	}

	// store the benchmark results as a file
	FILE* output_file = fopen("results.txt", "w");
	if (output_file == NULL) {
		printf("Error opening output file\n");
		fclose(models_file);
		return 1;
	}

	// test each model
	char model[32];
	while (fgets(model, sizeof(model), models_file) != NULL) {
		model[strcspn(model, "\n")] = 0;

		// benchmark timer start
		struct timespec start_time, end_time;
		clock_gettime(CLOCK_REALTIME, &start_time);

		// resource usage before the model runs
		printf("Now running: %s\n", model);
		fprintf(output_file, "Model: %s\n", model);
		fflush(output_file);
		fprintf(output_file, "Initial resource usage:\n");
		fflush(output_file);
		get_resource_usage(output_file);
	
		pid_t pid = fork();
		
		if (pid == -1) {
			printf("Error forking child process\n");
			fclose(models_file);
			fclose(output_file);
			return 1;
		}

		// run ollama in the child process
		if (pid == 0) {
			int status = execlp("ollama", "ollama", "run", model, "What is the most famous chord progression?", NULL);
			if (status == -1) {
				printf("Error executing ollama in the child process\n");
				return 1;
			}
		}

		// track results in the parent
		else {
			// wait for ollama to terminate
			int status;
			waitpid(pid, &status, 0);

			// stop the benchmark timer
			clock_gettime(CLOCK_REALTIME, &end_time);
			double benchmark_time = (end_time.tv_sec - start_time.tv_sec) + (end_time.tv_nsec - start_time.tv_nsec) / 1000000000.0;

			fprintf(output_file, "execution time: %.9f seconds\n", benchmark_time);
			fflush(output_file);

			// get the updated resource usage (usage at time of ollama termination)
			fprintf(output_file, "Post-prompt resource usage:\n");
			fflush(output_file);
			get_resource_usage(output_file);

			// output file styling
			fprintf(output_file, "\n--------------------------------\n");
			fflush(output_file);
		}
	}

	fclose(models_file);
	fclose(output_file);
	return 0;
}

/**
 * @brief measures current resource usage
 * @param output_file output stream to write resource usage results
 */
void get_resource_usage(FILE* output_file) {
	FILE* fptr;
	char buffer[PS_BUFFER_SIZE];

	// process status and process grep to search specifically for usage by ollama
	fptr = popen("ps -o %cpu,%mem -p $(pgrep -n ollama)", "r");
	if (fptr == NULL) {
		printf("Error running ps command\n");
		return;
	}

	// output the result of the resource usage
	while (fgets(buffer, sizeof(buffer), fptr) != NULL) {
		fprintf(output_file, "%s", buffer);
		fflush(output_file);
	}

	if (fclose(fptr) != 0) {
		printf("Error closing the file pointer\n");
	}
}
