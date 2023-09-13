/*
Copyright (c) 2022, NVIDIA CORPORATION. All rights reserved.

NVIDIA CORPORATION and its licensors retain all intellectual property
and proprietary rights in and to this software, related documentation
and any modifications thereto. Any use, reproduction, disclosure or
distribution of this software and related documentation without an express
license agreement from NVIDIA CORPORATION is strictly prohibited.
*/

#pragma region [  Core  ]

static void NRI_CALL SetCommandQueueDebugName(CommandQueue& commandQueue, const char* name)
{
    ((CommandQueueVK&)commandQueue).SetDebugName(name);
}

static void NRI_CALL QueueSubmit(CommandQueue& commandQueue, const QueueSubmitDesc& workSubmissionDesc)
{
    ((CommandQueueVK&)commandQueue).Submit(workSubmissionDesc);
}

#pragma endregion

#pragma region [  Helper  ]

static Result NRI_CALL ChangeResourceStates(CommandQueue& commandQueue, const TransitionBarrierDesc& transitionBarriers)
{
    return ((CommandQueueVK&)commandQueue).ChangeResourceStates(transitionBarriers);
}

static nri::Result NRI_CALL UploadData(CommandQueue& commandQueue, const TextureUploadDesc* textureUploadDescs, uint32_t textureUploadDescNum,
    const BufferUploadDesc* bufferUploadDescs, uint32_t bufferUploadDescNum)
{
    return ((CommandQueueVK&)commandQueue).UploadData(textureUploadDescs, textureUploadDescNum, bufferUploadDescs, bufferUploadDescNum);
}

static nri::Result NRI_CALL WaitForIdle(CommandQueue& commandQueue)
{
    return ((CommandQueueVK&)commandQueue).WaitForIdle();
}

#pragma endregion

Define_Core_CommandQueue_PartiallyFillFunctionTable(VK)
Define_Helper_CommandQueue_PartiallyFillFunctionTable(VK)
