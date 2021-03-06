/*=========================================================================

   Program: ParaView
   Module:    pqRemoteControlThread.h

   Copyright (c) 2005-2008 Sandia Corporation, Kitware Inc.
   All rights reserved.

   ParaView is a free software; you can redistribute it and/or modify it
   under the terms of the ParaView license version 1.2.

   See License_v1.2.txt for the full ParaView license.
   A copy of this license can be obtained by contacting
   Kitware Inc.
   28 Corporate Drive
   Clifton Park, NY 12065
   USA

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/
#ifndef _pqRemoteControlThread_h
#define _pqRemoteControlThread_h

#include <QThread>

class vtkRenderWindow;

/// A threaded server for the mobile remote control plugin.  The server can
/// send the ParaView render scene data to a client and receive camera state
/// information from the client.  This allows you to use a mobile device to
/// view the ParaView scene and control the ParaView camera.  The server runs
/// on a separate thread so that communication does not block the ParaView GUI.
class pqRemoteControlThread : public QThread
{
  Q_OBJECT

public:

  /// Stores camera state information. Bytes read from the socket will be written
  /// directly into an instance of this struct.
  struct CameraStateStruct
  {
    float Position[3];
    float FocalPoint[3];
    float ViewUp[3];
  };

  pqRemoteControlThread();
  ~pqRemoteControlThread();

  /// Open a server socket to listen on the given port.
  bool createServer(int port);

  /// Calls vtkServerSocket::WaitForConnection() with a timeout of 1 millisecond.
  /// Returns true if a new client connection was established.
  bool checkForConnection();

  /// Close the socket and cleanup.  If the remote control thread is running,
  /// it is not safe to call this method.  Instead, call shouldQuit() to tell the
  /// thread to stop, which will call close() for you when it terminates.
  void close();

  /// Notify the remote control thread that it should stop and return.  If a
  /// socket read/write is in progress, it might not quit immediately.
  void shouldQuit();

  /// Returns true if the server socket is open and waiting for a client connection.
  bool serverIsOpen();

  /// Returns true if there is an active socket connection with a client.
  bool clientIsConnected();

  /// Return a copy of the most recently received camera state struct.
  CameraStateStruct cameraState();

  /// Returns true new camera state information has been received since the
  /// last call to cameraState().
  bool hasNewCameraState();

  /// Export the scene in the given vtkRenderWindow.  This should only be
  /// called on the main GUI thread.  See requestExportScene().
  void exportScene(vtkRenderWindow* renderWindow);

  /// Integer commands used by the client and server.
  enum
  {
    READY_COMMAND = 1,
    SEND_METADATA_COMMAND = 2,
    SEND_OBJECTS_COMMAND = 3,
    RECEIVE_CAMERA_STATE_COMMAND = 4,
    HEARTBEAT_COMMAND = 5
  };


signals:

  /// Send a signal to the main GUI thread. The main GUI thread will
  /// then be responsible for calling exportScene().
  void requestExportScene();

protected:

  /// Main entry point for the thread loop.
  void run();

  /// Emits the requestExortScene() signal and blocks until exportScene()
  /// has been called by the main GUI thread.
  void exportSceneOnMainThread();

  /// NOTE:
  /// the following methods return true if everything is OK,
  /// they return false if there was an issue in the socket communication
  /// or if shouldQuit() has been called, indicating that the thread should stop.

  /// Blocks until there is new bytes to read on the client socket.
  bool waitForSocketActivity();

  /// Receive a integer from the client
  bool receiveCommand(int& command);

  /// Send an integer to the client.
  bool sendCommand(int command);

  /// A dispatch method to handle the received client command.
  bool handleCommand(int command);

  /// Receive a new camera state from the client
  bool receiveCameraState();

  /// Send scene metadata to the client.  Scene metadata is generated by a
  /// call to exportSceneOnMainThread().
  bool sendSceneInfo();

  /// Send data objects to the client.
  bool sendObjects();

private:

  class pqInternal;
  pqInternal* Internal;

};

#endif
