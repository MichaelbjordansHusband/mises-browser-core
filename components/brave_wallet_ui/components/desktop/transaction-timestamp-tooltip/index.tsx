/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

import * as React from 'react'

import {
  StyledWrapper,
  Tip,
  Pointer
} from './style'

export interface Props {
  children?: React.ReactNode
  text: string | React.ReactNode
}

function TransactionTimestampTooltip (props: Props) {
  const { children, text } = props
  const [active, setActive] = React.useState(false)

  
  // const showTip = () => {
  //   setActive(true)
  // }

  // const hideTip = () => {
  //   setActive(false)
  // }

  return (
    <StyledWrapper
      // onMouseEnter={showTip}
      // onMouseLeave={hideTip}
      onPress={() => setActive(!active)}
    >
      {children}
      {active && (
        <>
          <Pointer />
          <Tip>
            {text}
          </Tip>
        </>
      )}
    </StyledWrapper>
  )
}

export default TransactionTimestampTooltip
