/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2018
Author:      admin
Version:     1.0
Date:        2018-12-25 08:47:27
Description: caaimm
**************************************************/

#include "stdafx.h"

BM2F_ENTERACE(caai01_col)

int f_caai_start(CString dept_code, CString stats_period, CString begin_time, CString end_time, CDbConnection * conn);
int f_caai_allft(CString account_period, CString dept_code, CDbConnection * conn);
int f_caai01_col(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	int doFlag = 0;
	CDbCommand cmd(conn);
	CString sqlstr = " ";

	CString begin_time = "";
	CString end_time = "";
	CString dept_code = " ";
	CString whole_backlog_code = " ";

	CDbCommand cmd_inq(conn);

	int  v_total_count = 0;
	//系统的分页类信息。
	CPageInfo pageInfo;

	try
	{

		
		begin_time = bcls_rec->Tables[0].Rows[0]["BEGIN_TIME"].ToString().Substring(0,8);
		end_time = bcls_rec->Tables[0].Rows[0]["END_TIME"].ToString().Substring(0, 8);
		dept_code = bcls_rec->Tables[0].Rows[0]["DEPT_CODE"].ToString();
		
		//日期都存在判断范围一个月内
		if ((begin_time.Trim() != "") && (end_time.Trim() != "") && dept_code.Trim() != "")
		{
			CDateTime d1 = CDateTime::Parse(begin_time);
			CDateTime d2 = CDateTime::Parse(end_time);
			CTimeSpan timeSpan = d2 - d1;
			/*if (timeSpan.Days() > 30)
			{
				throw CApplicationException("日期范围超过一个月！");
				
			}
			else
			{*/
				for (int i1 = 0; i1 <=timeSpan.Days(); i1++)
				{
					//判断是否已经关账，如果已经关账则不进行核算
					sqlstr = " select CURRENT_STATUS from tcaac15"
						" where S_DATETIME<=@date"
						" and E_DATETIME>=@date"
						;
					cmd_inq.SetCommandText(sqlstr);
					cmd_inq.Parameters.Set("date", CDateTime::Parse(begin_time).AddDays(i1).ToString("yyyyMMdd"));
					cmd_inq.ExecuteReader();
					if (cmd_inq.Read())
					{
						if (cmd_inq.GetString(1) == "4")
						{
							sprintf(s.msg, "日期[%s]所在的会计期已经关账，不能核算，请重新选择日期!", (const char*)CDateTime::Parse(begin_time).AddDays(i1).ToString("yyyyMMdd"));
							
							throw CApplicationException(-1, s.msg, log.Location);
						}
					}
					cmd_inq.Close();

					doFlag = f_caai_start(dept_code, CDateTime::Parse(begin_time).AddDays(i1).ToString("yyyyMMdd"), CDateTime::Parse(begin_time).AddDays(i1).ToString("yyyyMMdd") + "000000", CDateTime::Parse(begin_time).AddDays(i1).ToString("yyyyMMdd") + "235959", conn);
					if (doFlag != 0)
					{
						strcpy(s.msg, "日成本核算失败!");
						throw CApplicationException(-1, s.msg, log.Location);
					}
				}
			/*}*/
		}

		doFlag = f_caai_allft(end_time.Substring(0,6),dept_code,conn);
		if (doFlag != 0)
		{
			strcpy(s.msg, "月分摊核算失败!");
			throw CApplicationException(-1, s.msg, log.Location);
		}

	}
	catch (CDbException& ex)
	{
		CFormattable arguments[] = { ex.GetCode(), ex.GetMsg() };
		CMessageFormat::Format(s.msg, "Database Error,sqlcode=[{0}],sqlmsg=[{1}]", arguments, 2);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg) - 1);
		s.flag = -1;
		doFlag = -1;
	}
	catch (CApplicationException& ex)
	{
		s.flag = ex.GetCode();
		strcpy(s.msg, ex.GetMsg());
		doFlag = -1;
	}
	catch (CException& ex)
	{
		strcpy(s.msg, ex.GetMsg());
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	return doFlag;
}


