# PID analysis for HypTPC using PyTorch

Using machine learning, identify proton, pion, kaon from momentum, energyloss, ToF.
Train/test data is created by Geant4 simulation
<br><br>

<dl>

## <dt>Enviornments</dt>

- chceck requirements.txt for details
- after creating new venv enviornment and activating、try pip install.

```.sh
$ python3 -m venv .venv
$ source .venv/bin/activate
(.venv) $ pip install -r requirements.txt
```

## <dt>How To Start</dt>

### <dd>Step1: Creating train/test Data

- Move to **geant** directory and start creating data for all each particles (Proton, Kaon, Pion)

```sh
$ cd geant
$ ./bin/Linux-g++/RCSim
$ /control/execute/vis.mac
$ /run/beamOn 350000
```

- This data will be saved as **rootfiles/(particle)\_raw.root**
- After creating data for all 3 particles, move to **create_rootfiles** and run **mktest.cc/mktrain.sh** to create test/train data for ML input.
- In these macro, truncated mean of the energyloss will be calculated and saved also with particle_id, tof and momentum.
- Inside **test.root/train.root**, data for each number of layer will be saved in individual trees (e.g. tree_xxlayer -> data for xx layer)
  <br><br>

</dd>

### <dd>Step2: Learning Process

- To start the training process, move to **learn/code** and run **train.sh**. **train_net.py** will run in bsub for each nunmber of layers.
- After training, run **test.sh** to evluate the traning model. The prediction-id and true-id will be filled to **output_xxlayer.root**

</dd>

### <dd>Step3: Comparision with Likelihood

- To Compare ML accuracy with likelihood method, move to **likelihood/ana** and run **accuracy.sh**. The accuracy result will be written in csv files.

</dd>

## <dt>Model</dt>

For details, check the attached pdf file.
<br><br>

## <dt>Results</dt>

For details, check the attached pdf file.

</dl>
