#include "include/utils.h"
#include "include/tensor.h"
#include <math.h>
#include <string.h>

//Evaluate accuracy for categorical tensors
double evaluate_accuracy(tensor* truth, tensor* prediction, int n_predictions)
{
    double score=0;
    for(int i=0;i<n_predictions;i++)
    {
        int* it_truth = arg_max(&truth[i]);
        int* it_prediction = arg_max(&prediction[i]);
        if(is_iterator_equal(it_truth,it_prediction, truth[i].shape->dimension))
        {
            score++;
        }
        free(it_truth);
        free(it_prediction);
    }
    return 100*score/n_predictions;
}

//Evaluate accuracy of a model using dataset features and labels_categorical
double evaluate_dataset_accuracy(dataset* data, model* model)
{
    tensor* predictions = model->predict(data->features, data->n_entries, model);
    double result = evaluate_accuracy(data->labels_categorical, predictions, data->n_entries);
    free_tensors(predictions, data->n_entries);
    return result;
}

//Get the background color code for a gray scale providing a value from 0 to 1
int get_background_color(double gray_scale)
{
    return 232 + (int)(gray_scale*24);
}

//Draw grayscale image from a tensor that might be 1D,2D or 3D using printf
void draw_image(tensor* img)
{
    int width;
    switch(img->shape->dimension)
    {
        case TwoD: width=img->shape->sizes[1]; break;
        case ThreeD: width=img->shape->sizes[2]; break;
        default: width= (int)sqrt(img->shape->sizes[0]);break;
    }
    int* iterator = get_iterator(img);
    int column=0;
    while(!img->is_done(img,iterator))
    {
        printf("\033[48;5;%dm  ", get_background_color(img->get_value(img,iterator)));
        if(++column==width)
        {
            column=0;
            printf("\033[0m\n");
        }
        iterator = img->get_next(img,iterator);
    }
}

//Format seconds to seconds string e.g 10-->'10s'
void to_seconds_string(char* string, long seconds)
{
    sprintf(string, "%lds", seconds);
}

void to_minutes_string(char* string, long seconds)
{
    long minutes = seconds/60;
    long remaining_seconds = seconds%60;
    char secs[4];
    to_seconds_string(secs, remaining_seconds);
    sprintf(string, "%ldm%s", minutes, secs);   
}

void to_hours_string(char* string, long seconds)
{
    long hours= seconds/3600;
    long remaining_seconds = seconds%3600;
    char mins[7];
    to_minutes_string(mins, remaining_seconds);
    sprintf(string, "%ldh%s", hours, mins);
}

void to_days_string(char* string, long seconds)
{
    long days = seconds/86400;
    long remaining_seconds = seconds%86400;
    char hours[10];
    to_hours_string(hours, remaining_seconds);
    sprintf(string, "%ldd%s", days, hours);
}

void seconds_to_string(char* string, long seconds)
{
    if(seconds<60)
    {
        to_seconds_string(string, seconds);
    }
    else if(seconds<3600)
    {
        to_minutes_string(string, seconds);
    }
    else if(seconds<86400)
    {
        to_hours_string(string, seconds);
    }
    else
    {
        to_days_string(string, seconds);
    }
}