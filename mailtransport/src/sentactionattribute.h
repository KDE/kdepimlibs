/*
  Copyright (C) 2010 Klarälvdalens Datakonsult AB,
      a KDAB Group company, info@kdab.net,
      author Tobias Koenig <tokoe@kdab.com>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
*/

#ifndef MAILTRANSPORT_SENTACTIONATTRIBUTE_H
#define MAILTRANSPORT_SENTACTIONATTRIBUTE_H

#include <mailtransport_export.h>

#include <QtCore/QSharedDataPointer>
#include <QtCore/QVariant>

#include <attribute.h>

namespace MailTransport
{

/**
 * @short An Attribute that stores the action to execute after sending.
 *
 * This attribute stores the action that will be executed by the mail dispatcher
 * after a mail has successfully be sent.
 *
 * @author Tobias Koenig <tokoe@kdab.com>
 * @since 4.6
 */
class MAILTRANSPORT_EXPORT SentActionAttribute : public Akonadi::Attribute
{
public:
    /**
     * @short A sent action.
     */
    class MAILTRANSPORT_EXPORT Action
    {
    public:
        /**
         * Describes the action type.
         */
        enum Type {
            Invalid,         ///< An invalid action.
            MarkAsReplied,   ///< The message will be marked as replied.
            MarkAsForwarded  ///< The message will be marked as forwarded.
        };

        /**
         * Describes a list of sent actions.
         */
        typedef QList<Action> List;

        /**
         * Creates a new invalid action.
         */
        Action();

        /**
         * Creates a new action.
         *
         * @param action The action that shall be executed.
         * @param value The action specific argument.
         */
        Action(Type type, const QVariant &value);

        /**
         * Creates an action from an @p other action.
         */
        Action(const Action &other);

        /**
         * Destroys the action.
         */
        ~Action();

        /**
         * Returns the type of the action.
         */
        Type type() const;

        /**
         * Returns the argument value of the action.
         */
        QVariant value() const;

        /**
         * @internal
         */
        Action &operator=(const Action &other);

        /**
         * @internal
         */
        bool operator==(const Action &other) const;

    private:
        //@cond PRIVATE
        class Private;
        QSharedDataPointer<Private> d;
        //@endcond
    };

    /**
     * Creates a new sent action attribute.
     */
    explicit SentActionAttribute();

    /**
     * Destroys the sent action attribute.
     */
    virtual ~SentActionAttribute();

    /**
     * Adds a new action to the attribute.
     *
     * @param type The type of the action that shall be executed.
     * @param value The action specific argument.
     */
    void addAction(Action::Type type, const QVariant &value);

    /**
     * Returns the list of actions.
     */
    Action::List actions() const;

    /* reimpl */
    SentActionAttribute *clone() const Q_DECL_OVERRIDE;
    QByteArray type() const Q_DECL_OVERRIDE;
    QByteArray serialized() const Q_DECL_OVERRIDE;
    void deserialize(const QByteArray &data) Q_DECL_OVERRIDE;

private:
    //@cond PRIVATE
    class Private;
    Private *const d;
    //@endcond
};

}

#endif
