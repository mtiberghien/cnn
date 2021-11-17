#include "include/layer.h"
#include "include/tensor.h"
#include <stdlib.h>

//Clear memory of temporary stored inputs and outputs
void clear_layer_input_output(layer* layer){
    for(int i=0;i<layer->batch_size;i++)
    {
        clear_tensor(layer->inputs[i]);
        clear_tensor(layer->outputs[i]);
        clear_tensor(layer->activation_inputs[i]);
    }
    free(layer->inputs);
    free(layer->outputs);
    free(layer->activation_inputs);
}

layer* build_layer_FC(int input_size, int output_size, activation* activation){
    //Allocate layer memory
    layer* result = (layer*)malloc(sizeof(layer));
    //Store input and output size
    result->input_size = input_size;
    result->output_size = output_size;
    //Initialize weights matrix which is input_sizeXoutput_size
    result->weights.size = input_size*output_size;
    for(int i=0;i<input_size*output_size;i++){
        result->weights.v[i] = (double)rand() / (double)RAND_MAX ;
    }
    //Initialize biases
    result->biases.size = output_size;
    result->biases.v = calloc(output_size,sizeof(double));
    //Set used methods for the layer
    result->forward_propagation_loop=forward_propagation_loop;
    result->backward_propagation_loop = backward_propagation_loop;
    result->forward_propagation=forward_propagation_FC;
    result->backward_propagation=backward_propagation_FC;
    //Store activation function
    result->activation = activation;
    //Return configured and initialized FC layer
    return result;
}

//Common forward propagation loop
tensor* forward_propagation_loop(tensor* inputs, int batch_size, struct layer* layer)
{
    //Store inputs (used in backward propagation)
    layer->inputs = inputs;
    layer->batch_size = batch_size;
    //Allocate memory for outputs and activation_input
    layer->outputs=malloc(sizeof(tensor)*batch_size);
    layer->activation_inputs=malloc(sizeof(tensor)*batch_size);
    // Loop into input batch
    for(int i=0;i<batch_size;i++)
    {
        //Output tensor memory allocation
        tensor output = layer->outputs[i];
        output.size = layer->output_size;
        output.v = calloc(layer->output_size,sizeof(double));

        tensor activation_input = layer->activation_inputs[i];
        activation_input.size = layer->output_size;
        activation_input.v = malloc(sizeof(double)*layer->output_size);
        //Execute specific forward propagation
        layer->forward_propagation(&inputs[i], &output, &activation_input, layer);
    }
    return layer->outputs; 
}

//Common backward propagation loop
tensor* backward_propagation_loop(tensor* output_errors, optimizer* optimizer, struct layer* layer)
{
    //Previous layer gradient error tensor memory allocation
    tensor* input_error = (tensor*)malloc(sizeof(tensor)*layer->batch_size);
    //Loop into following layer gradient error batch
    for(int i=0;i<layer->batch_size;i++)
    {
        //Output gradient error tensor
        tensor output_error = output_errors[i];
        //Previous layer gradient error tensor memory allocation
        tensor ie_t = input_error[i];
        ie_t.size = layer->input_size;
        ie_t.v = calloc(layer->input_size, sizeof(double));

        //Execute specific backward propagation
        layer->backward_propagation(&output_error, &ie_t, &layer->inputs[i], &layer->activation_inputs[i], optimizer, layer);
        //Clear gradient error from next layer
        clear_tensor(output_error);
    }
    // Clear memory
    free(output_errors);
    clear_layer_input_output(layer);
    return input_error;
}

//Forward propagation function for Fully Connected layer (perceptron)
tensor* forward_propagation_FC(tensor* input, tensor* output, tensor* activation_input, layer* layer){
    //Loop into output tensor
    for(int j=0;j<layer->output_size;j++)
    {
            //Loop into input tensor
            for(int k=0;k<layer->input_size;k++){
                //sum weighted input element using weights matrix
                output->v[j] += layer->weights.v[(j*layer->input_size)+k]* (input->v[k]);
            }
            //Add bias
            output->v[j] += layer->biases.v[j];
            //Store the activation input
            activation_input->v[j] = output->v[j];
            
    }
    //Execute activation function and return output tensor
    return layer->activation->activation_func(output);
}

//Backward propagation function for Fully Connected layer (perceptron)
tensor* backward_propagation_FC(tensor* output_error, tensor* input_error, tensor* input, tensor* activation_input, optimizer* optimizer, layer* layer)
{
    //Back propagate the gradient error tensor
    output_error = layer->activation->backward_propagation(activation_input,output_error, layer->activation);
    //Update biases using new output_error
    for(int j=0;j<layer->output_size;j++)
    {       
        //Optimizer update bias using the activation primitive
        layer->biases.v[j]=optimizer->apply_gradient(layer->biases.v[j], output_error->v[j], optimizer);           
    }
    //Calculate the gradient for previous layer and update weights
    for(int j=0;j<layer->input_size;j++){
        for(int k=0;k<layer->output_size;k++){
            //Calculate Previous layer gradient error
            input_error->v[j] += layer->weights.v[j+(layer->input_size*k)]*(output_error->v[k]);
            //Update weights
            layer->weights.v[k*layer->input_size+j]=optimizer->apply_gradient(layer->weights.v[k*layer->input_size+j], output_error->v[k]*input->v[j], optimizer);
        }
    }
    //Return gradient error tensor
    return input_error;
}