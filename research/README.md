# Puyo Chain Detector Research

Goals:
- Design the multilayer perceptron (MLP) Puyo color classifier to use in the C++ app.
- Define coordinate data for anchor regions in game footage.

## Setup

Depends on Python v3.9. Install most of the dependencies using pip:

```bash
pip install -r requirements.txt
```

Then install PyTorch from this link: https://pytorch.org/get-started/locally/

## Research

### Color Classification

- Run the notebook [Dataset Generation.ipynb](./Dataset%20Generation.ipynb) to generate `dataset-images.joblib` and `dataset-labels.joblib`.
- Then run the code in [Model Training](./Model%20Training.ipynb).
    - Outputs a `.pt` file for use in further Python projects, and an `.onnx` file for use in OpenCV's DNN module.
- Verify the model using [Model Testing](./Model%20Testing.ipynb).

Note to self: Once this is working, I need to go back and do some research on CVD filter compatibility.

### Field Regions

I mapped out different anchor points in gameplay images for both PPT2 and PPT1/PPe. You can see the whole process in [Regions PPT1 PPC.ipynb](./Regions%20PPT1%20PPC.ipynb) and [Regions PPT2.ipynb](./Regions%20PPT2.ipynb).

The results are in [regions-480-256.json](./regions-480-256.json). The region labelled "bonus" refers to the empty space that appears in the score region when a chain link pops.