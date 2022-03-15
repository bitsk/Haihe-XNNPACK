// Copyright 2022 Google LLC
//
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree.

#include <stdint.h>  // For size_t.

#include <xnnpack.h>
#include <xnnpack/log.h>
#include <xnnpack/params.h>
#include <xnnpack/subgraph.h>

static size_t calculate_batch_size(const struct xnn_value* input, size_t axis)
{
  size_t batch_size = 1;
  for (size_t i = 0; i < axis; i++) {
    batch_size *= input->shape.dim[i];
  }
  return batch_size;
}

static size_t calculate_input_stride(const struct xnn_value* input, size_t axis)
{
  size_t input_stride = 1;
  for (size_t i = axis; i < input->shape.num_dims; i++) {
    input_stride *= input->shape.dim[i];
  }
  return input_stride;
}

static enum xnn_status create_even_split_operator_helper(
    const struct xnn_node* node,
    size_t channels,
    size_t input_stride,
    size_t output_stride,
    struct xnn_operator_data* opdata,
    size_t index)
{
  switch (node->compute_type) {
    #ifndef XNN_NO_F16_OPERATORS
      case xnn_compute_type_fp16: {
        return xnn_create_copy_nc_x16(
            channels, input_stride, output_stride, node->flags, &opdata->operator_objects[index]);
      }
    #endif  // !defined(XNN_NO_F16_OPERATORS)
    case xnn_compute_type_fp32: {
      return xnn_create_copy_nc_x32(
          channels, input_stride, output_stride, node->flags, &opdata->operator_objects[index]);
    }
    #ifndef XNN_NO_QS8_OPERATORS
      case xnn_compute_type_qs8:
    #endif  // !defined(XNN_NO_QS8_OPERATORS)
    #ifndef XNN_NO_QU8_OPERATORS
      case xnn_compute_type_qu8:
    #endif  // !defined(XNN_NO_QU8_OPERATORS)
    #if !defined(XNN_NO_QS8_OPERATORS) || !defined(XNN_NO_QU8_OPERATORS)
      {
        return xnn_create_copy_nc_x8(
            channels, input_stride, output_stride, node->flags, &opdata->operator_objects[index]);
      }
    #endif  // !defined(XNN_NO_QS8_OPERATORS) || !defined(XNN_NO_QU8_OPERATORS)
    default:
      XNN_UNREACHABLE;
  }
}

static enum xnn_status create_even_split2_operator(
  const struct xnn_node* node,
  const struct xnn_value* values,
  size_t num_values,
  struct xnn_operator_data* opdata,
  struct xnn_code_cache* code_cache)
{
  assert(node->num_inputs == 1);
  const uint32_t input_id = node->inputs[0];
  assert(input_id != XNN_INVALID_VALUE_ID);
  assert(input_id < num_values);

  assert(node->num_outputs == 2);
  const uint32_t output1_id = node->outputs[0];
  assert(output1_id != XNN_INVALID_VALUE_ID);
  assert(output1_id < num_values);
  const uint32_t output2_id = node->outputs[1];
  assert(output2_id != XNN_INVALID_VALUE_ID);
  assert(output2_id < num_values);

  const size_t axis = node->params.even_split.axis;
  const size_t batch_size = calculate_batch_size(&values[input_id], axis);
  const size_t input_stride = calculate_input_stride(&values[input_id], axis);
  assert(input_stride % 2 == 0);
  const size_t channels = input_stride / 2;
  const size_t output_stride = channels;

  enum xnn_status status;
  status = create_even_split_operator_helper(node, channels, input_stride, output_stride, opdata, 0);
  if (status != xnn_status_success) {
    return status;
  }
  status = create_even_split_operator_helper(node, channels, input_stride, output_stride, opdata, 1);
  if (status != xnn_status_success) {
    return status;
  }

  opdata->inputs[0] = input_id;
  opdata->outputs[0] = output1_id;
  opdata->outputs[1] = output2_id;
  opdata->batch_size = batch_size;

  return status;
}

static enum xnn_status create_even_split3_operator(
  const struct xnn_node* node,
  const struct xnn_value* values,
  size_t num_values,
  struct xnn_operator_data* opdata,
  struct xnn_code_cache* code_cache)
{
  assert(node->num_inputs == 1);
  const uint32_t input_id = node->inputs[0];
  assert(input_id != XNN_INVALID_VALUE_ID);
  assert(input_id < num_values);

  assert(node->num_outputs == 3);
  const uint32_t output1_id = node->outputs[0];
  assert(output1_id != XNN_INVALID_VALUE_ID);
  assert(output1_id < num_values);
  const uint32_t output2_id = node->outputs[1];
  assert(output2_id != XNN_INVALID_VALUE_ID);
  assert(output2_id < num_values);
  const uint32_t output3_id = node->outputs[2];
  assert(output3_id != XNN_INVALID_VALUE_ID);
  assert(output3_id < num_values);

  const size_t axis = node->params.even_split.axis;
  const size_t batch_size = calculate_batch_size(&values[input_id], axis);
  const size_t input_stride = calculate_input_stride(&values[input_id], axis);
  assert(input_stride % 3 == 0);
  const size_t channels = input_stride / 3;
  const size_t output_stride = channels;

  enum xnn_status status;
  status = create_even_split_operator_helper(node, channels, input_stride, output_stride, opdata, 0);
  if (status != xnn_status_success) {
    return status;
  }
  status = create_even_split_operator_helper(node, channels, input_stride, output_stride, opdata, 1);
  if (status != xnn_status_success) {
    return status;
  }
  status = create_even_split_operator_helper(node, channels, input_stride, output_stride, opdata, 2);
  if (status != xnn_status_success) {
    return status;
  }

  opdata->inputs[0] = input_id;
  opdata->outputs[0] = output1_id;
  opdata->outputs[1] = output2_id;
  opdata->outputs[2] = output3_id;
  opdata->batch_size = batch_size;

  return status;
}

static enum xnn_status create_even_split4_operator(
  const struct xnn_node* node,
  const struct xnn_value* values,
  size_t num_values,
  struct xnn_operator_data* opdata,
  struct xnn_code_cache* code_cache)
{
  assert(node->num_inputs == 1);
  const uint32_t input_id = node->inputs[0];
  assert(input_id != XNN_INVALID_VALUE_ID);
  assert(input_id < num_values);

  assert(node->num_outputs == 4);
  const uint32_t output1_id = node->outputs[0];
  assert(output1_id != XNN_INVALID_VALUE_ID);
  assert(output1_id < num_values);
  const uint32_t output2_id = node->outputs[1];
  assert(output2_id != XNN_INVALID_VALUE_ID);
  assert(output2_id < num_values);
  const uint32_t output3_id = node->outputs[2];
  assert(output3_id != XNN_INVALID_VALUE_ID);
  assert(output3_id < num_values);
  const uint32_t output4_id = node->outputs[3];
  assert(output4_id != XNN_INVALID_VALUE_ID);
  assert(output4_id < num_values);

  const size_t axis = node->params.even_split.axis;
  const size_t batch_size = calculate_batch_size(&values[input_id], axis);
  const size_t input_stride = calculate_input_stride(&values[input_id], axis);
  assert(input_stride % 4 == 0);
  const size_t channels = input_stride / 4;
  const size_t output_stride = channels;

  enum xnn_status status;
  status = create_even_split_operator_helper(node, channels, input_stride, output_stride, opdata, 0);
  if (status != xnn_status_success) {
    return status;
  }
  status = create_even_split_operator_helper(node, channels, input_stride, output_stride, opdata, 1);
  if (status != xnn_status_success) {
    return status;
  }
  status = create_even_split_operator_helper(node, channels, input_stride, output_stride, opdata, 2);
  if (status != xnn_status_success) {
    return status;
  }
  status = create_even_split_operator_helper(node, channels, input_stride, output_stride, opdata, 3);
  if (status != xnn_status_success) {
    return status;
  }

  opdata->inputs[0] = input_id;
  opdata->outputs[0] = output1_id;
  opdata->outputs[1] = output2_id;
  opdata->outputs[2] = output3_id;
  opdata->outputs[3] = output4_id;
  opdata->batch_size = batch_size;

  return status;
}

static enum xnn_status setup_even_split_operator_helper(
  const size_t channels,
  const void* input_data,
  void* output_data,
  const struct xnn_operator_data* opdata,
  size_t index,
  pthreadpool_t threadpool)
{
  switch (opdata->operator_objects[0]->type) {
    #ifndef XNN_NO_F16_OPERATORS
      case xnn_operator_type_copy_nc_x16: {
        return xnn_setup_copy_nc_x16(
          opdata->operator_objects[index], opdata->batch_size, (const uint16_t*) input_data + index * channels,
          output_data, threadpool);
      }
    #endif  // !defined(XNN_NO_F16_OPERATORS)
    case xnn_operator_type_copy_nc_x32: {
      return xnn_setup_copy_nc_x32(
        opdata->operator_objects[index], opdata->batch_size, (const uint32_t*) input_data + index * channels,
        output_data, threadpool);
    }
    #if !defined(XNN_NO_QS8_OPERATORS) || !defined(XNN_NO_QU8_OPERATORS)
      case xnn_operator_type_copy_nc_x8: {
        return xnn_setup_copy_nc_x8(
          opdata->operator_objects[index], opdata->batch_size, (const uint8_t*) input_data + index * channels,
          output_data, threadpool);
      }
    #endif  // !defined(XNN_NO_QS8_OPERATORS) || !defined(XNN_NO_QU8_OPERATORS)
    default:
      XNN_UNREACHABLE;
  }
}

static enum xnn_status setup_even_split2_operator(
  const struct xnn_operator_data* opdata,
  const struct xnn_blob* blobs,
  size_t num_blobs,
  pthreadpool_t threadpool)
{
  const uint32_t input_id = opdata->inputs[0];
  assert(input_id != XNN_INVALID_VALUE_ID);
  assert(input_id < num_blobs);

  const uint32_t output1_id = opdata->outputs[0];
  assert(output1_id != XNN_INVALID_VALUE_ID);
  assert(output1_id < num_blobs);

  const uint32_t output2_id = opdata->outputs[1];
  assert(output2_id != XNN_INVALID_VALUE_ID);
  assert(output2_id < num_blobs);

  const struct xnn_blob* input_blob = blobs + input_id;
  const void* input_data = input_blob->data;
  assert(input_data != NULL);

  const struct xnn_blob* output1_blob = blobs + output1_id;
  void* output1_data = output1_blob->data;
  assert(output1_data != NULL);

  const struct xnn_blob* output2_blob = blobs + output2_id;
  void* output2_data = output2_blob->data;
  assert(output2_data != NULL);

  const size_t channels = opdata->operator_objects[0]->channels;

  enum xnn_status status = setup_even_split_operator_helper(channels, input_data, output1_data, opdata, 0, threadpool);
  if (status != xnn_status_success) {
    return status;
  }
  return setup_even_split_operator_helper(channels, input_data, output2_data, opdata, 1, threadpool);
}

static enum xnn_status setup_even_split3_operator(
  const struct xnn_operator_data* opdata,
  const struct xnn_blob* blobs,
  size_t num_blobs,
  pthreadpool_t
  threadpool)
{
  const uint32_t input_id = opdata->inputs[0];
  assert(input_id != XNN_INVALID_VALUE_ID);
  assert(input_id < num_blobs);

  const uint32_t output1_id = opdata->outputs[0];
  assert(output1_id != XNN_INVALID_VALUE_ID);
  assert(output1_id < num_blobs);

  const uint32_t output2_id = opdata->outputs[1];
  assert(output2_id != XNN_INVALID_VALUE_ID);
  assert(output2_id < num_blobs);

  const uint32_t output3_id = opdata->outputs[2];
  assert(output3_id != XNN_INVALID_VALUE_ID);
  assert(output3_id < num_blobs);

  const struct xnn_blob* input_blob = blobs + input_id;
  const void* input_data = input_blob->data;
  assert(input_data != NULL);

  const struct xnn_blob* output1_blob = blobs + output1_id;
  void* output1_data = output1_blob->data;
  assert(output1_data != NULL);

  const struct xnn_blob* output2_blob = blobs + output2_id;
  void* output2_data = output2_blob->data;
  assert(output2_data != NULL);

  const struct xnn_blob* output3_blob = blobs + output3_id;
  void* output3_data = output3_blob->data;
  assert(output3_data != NULL);

  const size_t channels = opdata->operator_objects[0]->channels;

  enum xnn_status status = setup_even_split_operator_helper(channels, input_data, output1_data, opdata, 0, threadpool);
  if (status != xnn_status_success) {
    return status;
  }
  status = setup_even_split_operator_helper(channels, input_data, output2_data, opdata, 1, threadpool);
  if (status != xnn_status_success) {
    return status;
  }
  return setup_even_split_operator_helper(channels, input_data, output3_data, opdata, 2, threadpool);
}

static enum xnn_status setup_even_split4_operator(
  const struct xnn_operator_data* opdata,
  const struct xnn_blob* blobs,
  size_t num_blobs,
  pthreadpool_t
  threadpool)
{
  const uint32_t input_id = opdata->inputs[0];
  assert(input_id != XNN_INVALID_VALUE_ID);
  assert(input_id < num_blobs);

  const uint32_t output1_id = opdata->outputs[0];
  assert(output1_id != XNN_INVALID_VALUE_ID);
  assert(output1_id < num_blobs);

  const uint32_t output2_id = opdata->outputs[1];
  assert(output2_id != XNN_INVALID_VALUE_ID);
  assert(output2_id < num_blobs);

  const uint32_t output3_id = opdata->outputs[2];
  assert(output3_id != XNN_INVALID_VALUE_ID);
  assert(output3_id < num_blobs);

  const uint32_t output4_id = opdata->outputs[3];
  assert(output4_id != XNN_INVALID_VALUE_ID);
  assert(output4_id < num_blobs);

  const struct xnn_blob* input_blob = blobs + input_id;
  const void* input_data = input_blob->data;
  assert(input_data != NULL);

  const struct xnn_blob* output1_blob = blobs + output1_id;
  void* output1_data = output1_blob->data;
  assert(output1_data != NULL);

  const struct xnn_blob* output2_blob = blobs + output2_id;
  void* output2_data = output2_blob->data;
  assert(output2_data != NULL);

  const struct xnn_blob* output3_blob = blobs + output3_id;
  void* output3_data = output3_blob->data;
  assert(output3_data != NULL);

  const struct xnn_blob* output4_blob = blobs + output4_id;
  void* output4_data = output4_blob->data;
  assert(output4_data != NULL);

  const size_t channels = opdata->operator_objects[0]->channels;

  enum xnn_status status = setup_even_split_operator_helper(channels, input_data, output1_data, opdata, 0, threadpool);
  if (status != xnn_status_success) {
    return status;
  }
  status = setup_even_split_operator_helper(channels, input_data, output2_data, opdata, 1, threadpool);
  if (status != xnn_status_success) {
    return status;
  }
  status = setup_even_split_operator_helper(channels, input_data, output3_data, opdata, 2, threadpool);
  if (status != xnn_status_success) {
    return status;
  }
  return setup_even_split_operator_helper(channels, input_data, output4_data, opdata, 3, threadpool);
}

enum xnn_status check_output_value(
  xnn_subgraph_t subgraph,
  size_t split_dim,
  uint32_t input_id,
  uint32_t output_id,
  const char* nth,
  enum xnn_node_type node_type)
{
  const struct xnn_value* input_value = &subgraph->values[input_id];
  const struct xnn_value* output_value = &subgraph->values[output_id];
  if (output_id >= subgraph->num_values) {
    xnn_log_error(
      "failed to define %s operator with %s output ID #%" PRIu32 ": invalid Value ID",
      xnn_node_type_to_string(node_type), nth, output_id);
    return xnn_status_invalid_parameter;
  }

  if (output_value->type != xnn_value_type_dense_tensor) {
    xnn_log_error(
      "failed to define %s operator with %s output ID #%" PRIu32
      ": unsupported Value type %d (expected dense tensor)",
      xnn_node_type_to_string(node_type), nth, output_id, output_value->type);
    return xnn_status_invalid_parameter;
  }

  if (input_value->shape.num_dims != output_value->shape.num_dims) {
    xnn_log_error(
      "failed to define %s operator with %s output ID #%" PRIu32
      ": mismatch number of dimensions, input has %zu, %s output has %zu",
      xnn_node_type_to_string(node_type), nth, output_id, input_value->shape.num_dims,
      nth, output_value->shape.num_dims);
    return xnn_status_invalid_parameter;
  }

  for (size_t i = 0; i < input_value->shape.num_dims; i++) {
    if (i != split_dim && input_value->shape.dim[i] != output_value->shape.dim[i]) {
      xnn_log_error(
        "failed to define %s operator with %s output ID #%" PRIu32
        ": mismatch dimension %zu, %s output has %zu, input has %zu",
        xnn_node_type_to_string(node_type), nth, output_id, i, nth, output_value->shape.dim[i],
        input_value->shape.dim[i]);
      return xnn_status_invalid_parameter;
    }
  }

  if (input_value->datatype != output_value->datatype) {
    xnn_log_error(
      "failed to define %s operator with input ID #%" PRIu32 " and %s output ID #%" PRIu32
      ": mismatching datatypes across the input (%s), the %s output (%s)",
      xnn_node_type_to_string(node_type), input_id, nth, output_id,
      xnn_datatype_to_string(input_value->datatype), nth, xnn_datatype_to_string(output_value->datatype));
    return xnn_status_invalid_parameter;
  }

  return xnn_status_success;
}

#if !defined(XNN_NO_QS8_OPERATORS) || !defined(XNN_NO_QU8_OPERATORS)
enum xnn_status check_output_compute_type(
  xnn_subgraph_t subgraph,
  uint32_t input_id,
  uint32_t output_id,
  const char* nth,
  enum xnn_node_type node_type)
{
  const struct xnn_value* input_value = &subgraph->values[input_id];
  const struct xnn_value* output_value = &subgraph->values[output_id];
  if (input_value->quantization.zero_point != output_value->quantization.zero_point) {
    xnn_log_error(
      "failed to define %s operator with input ID #%" PRIu32 " and output ID #%" PRIu32
      ": mismatching quantization zero point across the input (%d) and the %s output (%d)",
      xnn_node_type_to_string(node_type), input_id, output_id,
      input_value->quantization.zero_point, nth, output_value->quantization.zero_point);
    return xnn_status_invalid_parameter;
  }
  if (input_value->quantization.scale != output_value->quantization.scale) {
    xnn_log_error(
      "failed to define %s operator with input ID #%" PRIu32 " and output ID #%" PRIu32
      ": mismatching quantization scale across the input (%.7g) and the %s output (%.7g)",
      xnn_node_type_to_string(node_type), input_id, output_id, input_value->quantization.scale,
      nth, output_value->quantization.scale);
    return xnn_status_invalid_parameter;
  }
  return xnn_status_success;
}
#endif  // !defined( XNN_NO_QS8_OPERATORS) || !defined(XNN_NO_QU8_OPERATORS)

enum xnn_status xnn_define_even_split_n(
  enum xnn_node_type node_type,
  xnn_subgraph_t subgraph,
  size_t split_dim,
  uint32_t input_id,
  size_t num_outputs,
  const uint32_t* output_ids,
  uint32_t flags)
{
  assert(num_outputs > 1);
  assert(num_outputs < 5);

  if ((xnn_params.init_flags & XNN_INIT_FLAG_XNNPACK) == 0) {
    xnn_log_error(
      "failed to define %s operator: XNNPACK is not initialized", xnn_node_type_to_string(node_type));
    return xnn_status_uninitialized;
  }

  // Check input.
  if (input_id >= subgraph->num_values) {
    xnn_log_error(
      "failed to define %s operator with the input ID #%" PRIu32 ": invalid Value ID",
      xnn_node_type_to_string(node_type), input_id);
    return xnn_status_invalid_parameter;
  }

  const struct xnn_value* input_value = &subgraph->values[input_id];
  if (input_value->type != xnn_value_type_dense_tensor) {
    xnn_log_error(
      "failed to define %s operator with the input ID #%" PRIu32
      ": unsupported Value type %d (expected dense tensor)",
      xnn_node_type_to_string(node_type), input_id, input_value->type);
    return xnn_status_invalid_parameter;
  }

  check_output_value(subgraph, split_dim, input_id, output_ids[0], "first", node_type);
  check_output_value(subgraph, split_dim, input_id, output_ids[1], "second", node_type);

  if (num_outputs > 2) {
    check_output_value(subgraph, split_dim, input_id, output_ids[2], "third", node_type);
  }
  if (num_outputs > 3) {
    check_output_value(subgraph, split_dim, input_id, output_ids[3], "fourth", node_type);
  }

  // Check that the split dimension can be evenly split into outputs.
  if (split_dim >= input_value->shape.num_dims) {
    xnn_log_error(
      "failed to define %s operator with the input ID #%" PRIu32
      ": split dimension (%zu) exceeds the number of dimensions (%zu)",
      xnn_node_type_to_string(node_type), input_id, split_dim, input_value->shape.num_dims);
    return xnn_status_invalid_parameter;
  }

  if (input_value->shape.dim[split_dim] % num_outputs != 0) {
    xnn_log_error(
      "failed to define %s operator with the input ID #%" PRIu32
      ": split dimension %zu has value %zu which cannot be evenly split into %zu",
      xnn_node_type_to_string(node_type), input_id, split_dim, input_value->shape.dim[split_dim], num_outputs);
    return xnn_status_invalid_parameter;
  }

  // Check that the split dimensions of output add up;
  size_t output_dimensions_sum = 0;
  for (size_t i = 0; i < num_outputs; i++) {
    const struct xnn_value* output_value = &subgraph->values[output_ids[i]];
    output_dimensions_sum += output_value->shape.dim[split_dim];
  }

  if (output_dimensions_sum != input_value->shape.dim[split_dim]) {
    xnn_log_error(
      "failed to define %s operator with the input ID #%" PRIu32
      ": input split dimension value (%zu) does not match the sum of output split dimensions value %zu",
      xnn_node_type_to_string(node_type), input_id, input_value->shape.dim[split_dim], output_dimensions_sum);
    return xnn_status_invalid_parameter;
  }

  enum xnn_compute_type compute_type = xnn_compute_type_invalid;
  switch (input_value->datatype) {
#ifndef XNN_NO_F16_OPERATORS
    case xnn_datatype_fp16:
      compute_type = xnn_compute_type_fp16;
      break;
#endif  // !defined(XNN_NO_F16_OPERATORS)
    case xnn_datatype_fp32:
      compute_type = xnn_compute_type_fp32;
      break;
#ifndef XNN_NO_QS8_OPERATORS
    case xnn_datatype_qint8:
      compute_type = xnn_compute_type_qs8;
      break;
#endif  // !defined(XNN_NO_QS8_OPERATORS)
#ifndef XNN_NO_QU8_OPERATORS
    case xnn_datatype_quint8:
      compute_type = xnn_compute_type_qu8;
      break;
#endif  // !defined(XNN_NO_QU8_OPERATORS)
    default:
      xnn_log_error(
        "failed to define %s operator with input ID #%" PRIu32 ": unsupported Value datatype %s (%d)",
        xnn_node_type_to_string(node_type), input_id, xnn_datatype_to_string(input_value->datatype),
        input_value->datatype);
      return xnn_status_invalid_parameter;
  }

  #if !defined(XNN_NO_QS8_OPERATORS) || !defined(XNN_NO_QU8_OPERATORS)
    if (compute_type == xnn_compute_type_qs8 || compute_type == xnn_compute_type_qu8) {
      check_output_compute_type(subgraph, input_id, output_ids[0], "first", node_type);
      check_output_compute_type(subgraph, input_id, output_ids[1], "second", node_type);
      if (num_outputs > 2) {
        check_output_compute_type(subgraph, input_id, output_ids[2], "third", node_type);
      }
      if (num_outputs > 3) {
        check_output_compute_type(subgraph, input_id, output_ids[3], "fourth", node_type);
      }
    }
  #endif  // !defined( XNN_NO_QS8_OPERATORS) || !defined(XNN_NO_QU8_OPERATORS)


  struct xnn_node* node = xnn_subgraph_new_node(subgraph);
  if (node == NULL) {
    return xnn_status_out_of_memory;
  }

  node->params.even_split.axis = split_dim;
  node->type = xnn_node_type_even_split2;
  node->compute_type = compute_type;
  node->num_inputs = 1;
  node->inputs[0] = input_id;
  node->num_outputs = num_outputs;
  node->outputs[0] = output_ids[0];
  node->outputs[1] = output_ids[1];
  switch (num_outputs) {
    case 2:
      node->create = create_even_split2_operator;
      node->setup = setup_even_split2_operator;
      break;
    case 3:
      node->outputs[2] = output_ids[2];
      node->create = create_even_split3_operator;
      node->setup = setup_even_split3_operator;
      break;
    case 4:
      node->outputs[2] = output_ids[2];
      node->outputs[3] = output_ids[3];
      node->create = create_even_split4_operator;
      node->setup = setup_even_split4_operator;
      break;
    default:
      XNN_UNREACHABLE;
  }
  node->flags = flags;

  return xnn_status_success;
};

enum xnn_status xnn_define_even_split2(
  xnn_subgraph_t subgraph,
  size_t split_dim,
  uint32_t input_id,
  uint32_t output1_id,
  uint32_t output2_id,
  uint32_t flags)
{
  const uint32_t output_ids[2] = { output1_id, output2_id };
  return xnn_define_even_split_n(
    xnn_node_type_even_split2, subgraph, split_dim, input_id, XNN_COUNT_OF(output_ids), output_ids, flags);
}

enum xnn_status xnn_define_even_split3(
  xnn_subgraph_t subgraph,
  size_t split_dim,
  uint32_t input_id,
  uint32_t output1_id,
  uint32_t output2_id,
  uint32_t output3_id,
  uint32_t flags)
{
  const uint32_t output_ids[3] = { output1_id, output2_id, output3_id };
  return xnn_define_even_split_n(
    xnn_node_type_even_split3, subgraph, split_dim, input_id, XNN_COUNT_OF(output_ids), output_ids, flags);
}

enum xnn_status xnn_define_even_split4(
  xnn_subgraph_t subgraph,
  size_t split_dim,
  uint32_t input_id,
  uint32_t output1_id,
  uint32_t output2_id,
  uint32_t output3_id,
  uint32_t output4_id,
  uint32_t flags)
{
  const uint32_t output_ids[4] = { output1_id, output2_id, output3_id, output4_id };
  return xnn_define_even_split_n(
    xnn_node_type_even_split4, subgraph, split_dim, input_id, XNN_COUNT_OF(output_ids), output_ids, flags);
}