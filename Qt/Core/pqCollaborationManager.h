/*=========================================================================

   Program: ParaView
   Module:    pqCollaborationManager.h

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
#ifndef _pqCollaborationManager_h
#define _pqCollaborationManager_h

#include "pqCoreExport.h"
#include "vtkSMMessageMinimal.h"
#include <QObject>

class pqServer;
class pqView;
class QSignalMapper;

/// pqCollaborationManager is a QObject that aims to handle the collaboration
/// for the Qt layer. This class is used to synchronize the ActiveObject across
/// client instances as well as managing the rendering request when data has
/// been changed by other clients.
/// This class is responsible to synchronize
///    - rendering request
///    - pqProxy internal state
///    - master/slave (enable/disable edition control in UI)
///    - selected active source

class PQCORE_EXPORT pqCollaborationManager : public  QObject
{
  Q_OBJECT
public:  
  pqCollaborationManager(QObject* parent);
  virtual ~pqCollaborationManager();
  void setServer(pqServer*);

  /// Return your userId regarding your server connection
  int userId();

  /// Request the server to provide the list of user ids connected to the server
  /// as well as requesting to the clients their names.
  void updateUserList();

  /// Return the number of connected users to the server
  int getNumberOfUsers();

  /// Return the name of a given user based on its id
  QString getUserName(int userId);

  /// Return the idx connected userId
  int getUserId(int idx);

signals:
  /// This will be triggered locally to broadcast the request
  void triggerRender(int viewId);

  /// This will be triggered by the remote clients to update any interessting
  /// components. This should be triggered by local client to broadcast to
  /// the other clients
  void triggerChatMessage(int userId, QString& msgContent);

  /// This will be triggered by remote clients as well as local clients.
  /// It is triggered by local clients in order to notify the remote ones
  /// with our current last user name.
  void triggerUpdateUser(int userId, QString& userName, bool requestUpdateFromOthers);

  /// This will be triggered after each refreshUserList() if the user list has
  /// really changed
  void triggerUpdateUserList();

public slots:

  /// This will update the user information based on the latest server status
  /// This slot is called internally every 5s
  void refreshUserList();

  /// This will attach to the provided view the necessary listeners to share
  /// collaborative actions such as rendering decision, ...
  void addCollaborationEventManagement(pqView*);
  void removeCollaborationEventManagement(pqView*);

  /// This will be triggered by the triggerChatMessage() signal and will
  /// broadcast to other client a chat message
  void onChatMessage(int userId, QString& msgContent);

  /// This will be triggered by the updateParticipant() signal and will
  /// broadcast to other client the current user name and if we expect other
  /// to do the same or not
  void onUpdateUser(int userId, QString& userName, bool requestUpdateFromOthers);

private slots:
  /// Called when a message has been sent by another client
  /// This method will trigger signals that will be used by other Qt classes
  /// to synchronize their state.
  void onClientMessage(vtkSMMessage* msg);

  /// This will be triggered by the triggerRender(int) signal and will
  /// broadcast to other client a render request
  void onTriggerRender(int viewId);

  /// This will call force render on all the renderer that needs to be render
  void render();

private:
  pqCollaborationManager(const pqCollaborationManager&);  // Not implemented.
  pqCollaborationManager& operator=(const pqCollaborationManager&); // Not implemented.

  class pqInternals;
  pqInternals* Internals;
  QSignalMapper* viewsSignalMapper;
};

#endif // !_pqCollaborationManager_h
