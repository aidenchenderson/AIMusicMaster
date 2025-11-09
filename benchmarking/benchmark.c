#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#define PS_BUFFER_SIZE 1024

int get_max_resource_usage(float* max_cpu, float* max_mem);

/**
 * @brief program to benchmark ollama models on the RPi5 16GB
 * @param argc input argument count
 * @param argv input argument vector
 */
int main(int argc, char** argv) {
	// open the file with the list of models to be tested
	FILE* models_file = fopen("models.txt", "r");
	if (models_file == NULL) {
		printf("Error opening models file\n");
		return 1;
	}

	// open the file where benchmark results will be saved
	FILE* output_file = fopen("results.txt", "w");
	if (output_file == NULL) {
		printf("Error opening output file\n");
		fclose(models_file);
		return 1;
	}

	// model name and prompt
	char model[32];
	char* prompt = "You are a guitar assistant. Describe the most common mistakes new guitar players make in approximately 200 words.";	

	// test each model
	while (fgets(model, sizeof(model), models_file) != NULL) {
		model[strcspn(model, "\n")] = 0;

		// benchmark timer start
		struct timespec start_time, end_time;
		clock_gettime(CLOCK_REALTIME, &start_time);

		printf("Now running: %s\n", model);
		fprintf(output_file, "Model: %s\n", model);
		fflush(output_file);

		pid_t pid = fork();

		if (pid < 0) {
			printf("Error creating child process\n");
			fclose(models_file);
			fclose(output_file);
			return 1;
		}

		// run ollama in the child process
		if (pid == 0) {
			// reroute stdout to a response file to save the LLM responses.
			if (freopen("responses.txt", "a+", stdout) == NULL) {
				printf("Error redirecting stdout to responses.txt\n");
				return 1;
			}

			printf("response from %s\n", model);
			fflush(stdout);

			int status = execlp("ollama", "ollama", "run", model, prompt, NULL);
			if (status == -1) {
				printf("Error executing ollama in the child process\n");
				return 1;
			}
		}

		// track results in the parent
		else {
			float max_cpu = 0.0, max_mem = 0.0;

			// periodically check the resource usage and store the max
			do {
				int status;
				pid_t wait_result = waitpid(pid, &status, WNOHANG);
				if (wait_result == pid) break;

				int usage_result = get_max_resource_usage(&max_cpu, &max_mem);
				if (usage_result != 0) {
					printf("Error getting the max resource usage.\n");
					if (kill(pid, SIGKILL) == -1) printf("Error terminating child process\n");
					fclose(models_file);
					fclose(output_file);
				}

				sleep(5);
			} while (1);

			// wait for ollama to terminate
			int status;
			waitpid(pid, &status, 0);

			// stop the benchmark timer
			clock_gettime(CLOCK_REALTIME, &end_time);
			double benchmark_time = (end_time.tv_sec - start_time.tv_sec) + (end_time.tv_nsec - start_time.tv_nsec) / 1000000000.0;

			fprintf(output_file, "execution time: %.9f seconds\n", benchmark_time);
			fflush(output_file);

			fprintf(output_file, "max CPU usage: %.2f%%\n", max_cpu);
			fprintf(output_file, "max mem usage: %.2f%%\n", max_mem);
			fflush(output_file);

			// output file styling
			fprintf(output_file, "--------------------------------\n");
			fflush(output_file);
		}
	}

	fclose(models_file);
	fclose(output_file);
	return 0;
}

/**
 * @brief measures resource usage during execution, storing the max cpu and memory usage
 * @param max_cpu pointer to the current maximum cpu usage
 * @param max_cpu pointer to the current maximum memory usage
 */
int get_max_resource_usage(float* max_cpu, float* max_mem) {
	FILE* fptr;
	char buffer[PS_BUFFER_SIZE];
	float curr_cpu = 0.0, curr_mem = 0.0;

	// process status and process grep to search specifically for usage by ollama
	fptr = popen("ps -o %cpu,%mem -p $(pgrep -n ollama)", "r");
	if (fptr == NULL) {
		printf("Error running ps command\n");
		return 1;
	}

	// output the result of the resource usage
	while (fgets(buffer, sizeof(buffer), fptr) != NULL) {
		if (sscanf(buffer, "%f %f", &curr_cpu, &curr_mem) == 2) {
			if (max_cpu && curr_cpu > *max_cpu) {
				*max_cpu = curr_cpu;
			}
			if (max_mem && curr_mem > *max_mem) {
				*max_mem = curr_mem;
			}
		}
	}

	if (fclose(fptr) != 0) {
		printf("Error closing the file pointer\n");
		return 1;
	}

	return 0;
}
